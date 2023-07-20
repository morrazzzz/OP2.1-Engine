#pragma once

#define CMD0(cls)					{ static cls x##cls();				Console->AddCommand(&x##cls);}
#define CMD1(cls,p1)				{ static cls x##cls(p1);			Console->AddCommand(&x##cls);}
#define CMD2(cls,p1,p2)				{ static cls x##cls(p1,p2);			Console->AddCommand(&x##cls);}
#define CMD3(cls,p1,p2,p3)			{ static cls x##cls(p1,p2,p3);		Console->AddCommand(&x##cls);}
#define CMD4(cls,p1,p2,p3,p4)		{ static cls x##cls(p1,p2,p3,p4);	Console->AddCommand(&x##cls);}

ENGINE_API extern	xr_vector<xr_token>	vid_font_profile_tokens;
ENGINE_API extern	xr_vector<xr_token>	languages_tokens;

class ENGINE_API	IConsole_Command
{
public		:
	friend class	CConsole;
	typedef char	TInfo	[256];
	typedef char	TStatus	[256];
protected	:
	LPCSTR			cName;
	bool			bEnabled;
	bool			bLowerCaseArgs;
	bool			bEmptyArgsHandled;

	IC	bool		EQ(LPCSTR S1, LPCSTR S2) { return xr_strcmp(S1,S2)==0; }
	IC bool stateArg(LPCSTR args)
	{
		if (EQ(args, "on"))		return true;
		if (EQ(args, "1"))		return true;
		if (EQ(args, "off"))	return false;
		if (EQ(args, "0"))		return false;
		InvalidSyntax();
		return false;
	}
public		:
	IConsole_Command		(LPCSTR N) : 
	  cName				(N),
	  bEnabled			(TRUE),
	  bLowerCaseArgs	(TRUE),
	  bEmptyArgsHandled	(FALSE)
	{};
	virtual ~IConsole_Command()
	{
		if(Console)
			Console->RemoveCommand(this);
	};

	LPCSTR			Name()			{ return cName;	}

	void			InvalidSyntax() {
		TInfo I; Info(I);
		Msg("~ Invalid syntax in call to '%s'",cName);
		Msg("~ Valid arguments: %s", I);
	}
	virtual void	Execute	(LPCSTR args)	= 0;
	virtual void	Status	(TStatus& S)	{ S[0]=0; }
	virtual void	Info	(TInfo& I)		{ strcpy_s(I,"no arguments"); }
	virtual void	Save	(IWriter *F)	{
		TStatus		S;	Status(S);
		if (S[0])	F->w_printf("%s %s\r\n",cName,S); 
	}
};

class ENGINE_API CCC_VectorToken : public IConsole_Command
{
protected:
	u32* value;
	xr_vector<xr_token>* tokens;
public:
	CCC_VectorToken(LPCSTR N, u32* V, xr_vector<xr_token>* T) :IConsole_Command(N), value(V), tokens(T)
	{
		int i = 0;
	};

	void Info(TInfo& I) override
	{
		string256 buf;
		std::fill(std::begin(buf), std::end(buf), '\0');
		std::for_each(tokens->begin(), tokens->end(), [&](xr_token token)
		{
			bool bufEmpty = xr_strlen(buf) == 0;
			sprintf_s(buf, "%s%s%s", bufEmpty ? "" : buf, bufEmpty ? "" : "/", token.name);
		});
		strcpy_s(I, buf);
	};

	void Execute(LPCSTR args) override
	{
		auto finder = std::find_if(tokens->begin(), tokens->end(), [&](xr_token token)
		{
			return xr_strcmp(token.name, args) == 0;
		});
		if (finder != tokens->end())
			*value = (*finder).id;
		else
		{
			InvalidSyntax();
			*value = 0;
		}
	}

	void Save(IWriter* F) override
	{
		TStatus	S;
		Status(S);
		if (xr_strcmp(S, "?") == 0)
		{
			InvalidSyntax();
			return;
		}
		F->w_printf("%s %s\r\n", cName, S);
	};

	void Status(TStatus& S) override
	{
		if (*value==static_cast<u32>(-1) || tokens->size()==0)
			strcpy_s(S, "?");
		else
		{
			auto finder = std::find_if(tokens->begin(), tokens->end(), [&](xr_token token)
			{
				return token.id == static_cast<int>(*value);
			});
			if (finder != tokens->end())
				strcpy_s(S, (*finder).name);
			else
				strcpy_s(S, "?");
		}
	}

	xr_vector<xr_token>* GetVectorTokens() const
	{
		return tokens;
	}

	xr_token* GetToken() const
	{
		Msg("! DO NOT USE THIS CALL DIRECTLY! Use alternative GetVectorTokens() for this class");
		return nullptr;// &*tokens->begin();
	}
};

class ENGINE_API CCC_Bool:public IConsole_Command
{
protected:
	bool *value;
public:
	CCC_Bool(LPCSTR N, bool* value): IConsole_Command(N), value(value) {}
	const BOOL GetValue()const { return *value; }
	void Execute(LPCSTR args) override 
	{
		if (EQ(args,"on"))			*value=true;
		else if (EQ(args,"off"))	*value=false;
		else if (EQ(args,"1"))		*value=true;
		else if (EQ(args,"0"))		*value=false;
		else InvalidSyntax();
	};

	void	Status	(TStatus& S) override
	{
		strcpy_s(S,*value?"on":"off");
	}

	void	Info	(TInfo& I) override
	{
		strcpy_s(I,"'on/off' or '1/0'");
	}
};

class ENGINE_API	CCC_Mask : public IConsole_Command
{
protected	:
	Flags32*	value;
	u32			mask;
public		:
	CCC_Mask(LPCSTR N, Flags32* V, u32 M) :  IConsole_Command(N),  value(V), mask(M)
	{
	
	}
	  const BOOL GetValue()const{ return value->test(mask); }
	virtual void	Execute	(LPCSTR args)
	{
		if (EQ(args,"on"))			value->set(mask,TRUE);
		else if (EQ(args,"off"))	value->set(mask,FALSE);
		else if (EQ(args,"1"))		value->set(mask,TRUE);
		else if (EQ(args,"0"))		value->set(mask,FALSE);
		else InvalidSyntax();
	}
	virtual void	Status	(TStatus& S)
	{	strcpy_s(S,value->test(mask)?"on":"off"); }
	virtual void	Info	(TInfo& I)
	{	strcpy_s(I,"'on/off' or '1/0'"); }
};

class ENGINE_API CCC_MaskStat :public CCC_Mask
{
public:
	CCC_MaskStat(LPCSTR N, Flags32* V, u32 M) : CCC_Mask(N, V, M)
	{
	}

	void	Execute(LPCSTR args) override
	{
		bool state = stateArg(args);
		if (mask == rsStatistic && state)
			value->set(rsStatisticFPS, FALSE);
		else if (mask == rsStatisticFPS && state)
			value->set(rsStatistic, FALSE);
		if (!state)
		{
			value->set(rsStatistic, FALSE);
			value->set(rsStatisticFPS, FALSE);
			return;
		}
		value->set(mask, state ? TRUE : FALSE);
		//CCC_Mask::Execute(args);
	}
};

class ENGINE_API	CCC_ToggleMask : public IConsole_Command
{
protected	:
	Flags32*	value;
	u32			mask;
public		:
	CCC_ToggleMask(LPCSTR N, Flags32* V, u32 M) :
	  IConsole_Command(N),
	  value(V),
	  mask(M)
	{bEmptyArgsHandled=TRUE;};
	  const BOOL GetValue()const{ return value->test(mask); }
	virtual void	Execute	(LPCSTR args)
	{
		value->set(mask,!GetValue());
		TStatus S;
		strconcat(sizeof(S),S,cName," is ", value->test(mask)?"on":"off");
		Log(S);
	}
	virtual void	Status	(TStatus& S)
	{	strcpy_s(S,value->test(mask)?"on":"off"); }
	virtual void	Info	(TInfo& I)
	{	strcpy_s(I,"'on/off' or '1/0'"); }
};

class ENGINE_API	CCC_Token : public IConsole_Command
{
protected	:
	u32*			value;
	xr_token*		tokens;
public		:
	CCC_Token(LPCSTR N, u32* V, xr_token* T) :
	  IConsole_Command(N),
	  value(V),
	  tokens(T)
	{};

	virtual void	Execute	(LPCSTR args)
	{
		xr_token* tok = tokens;
		while (tok->name) {
			if (_stricmp(tok->name,args)==0) {
				*value=tok->id;
				break;
			}
			tok++;
		}
		if (!tok->name) InvalidSyntax();
	}
	virtual void	Status	(TStatus& S)
	{
		xr_token *tok = tokens;
		while (tok->name) {
			if (tok->id==(int)(*value)) {
				strcpy_s(S,tok->name);
				return;
			}
			tok++;
		}
		strcpy_s(S,"?");
		return;
	}
	virtual void	Info	(TInfo& I)
	{	
		I[0]=0;
		xr_token *tok = tokens;
		while (tok->name) {
			if (I[0]) strcat(I,"/");
			strcat(I,tok->name);
			tok++;
		}
	}
	virtual xr_token* GetToken(){return tokens;}
};

class ENGINE_API	CCC_Float : public IConsole_Command
{
protected	:
	float*			value;
	float			min,max;
public		:
	CCC_Float(LPCSTR N, float* V, float _min=0, float _max=1) :
	  IConsole_Command(N),
	  value(V),
	  min(_min),
	  max(_max)
	{};
	  const float	GetValue	() const {return *value;};
	  const float	GetMin		() const {return min;};
	  const float	GetMax		() const {return max;};

	virtual void	Execute	(LPCSTR args)
	{
		float v = float(atof(args));
		if (v<(min-EPS) || v>(max+EPS) ) InvalidSyntax();
		else	*value = v;
	}
	virtual void	Status	(TStatus& S)
	{	
		sprintf_s	(S,sizeof(S),"%3.5f",*value);
		while	(xr_strlen(S) && ('0'==S[xr_strlen(S)-1]))	S[xr_strlen(S)-1] = 0;
	}
	virtual void	Info	(TInfo& I)
	{	
		sprintf_s(I,sizeof(I),"float value in range [%3.3f,%3.3f]",min,max);
	}
};

class ENGINE_API	CCC_Vector3 : public IConsole_Command
{
protected	:
	Fvector*		value;
	Fvector			min,max;
public		:
	CCC_Vector3(LPCSTR N, Fvector* V, const Fvector _min, const Fvector _max) :
	  IConsole_Command(N),
	  value(V)
	{
		min.set(_min);
		max.set(_max);
	};

	virtual void	Execute	(LPCSTR args)
	{
		Fvector v;
		if (3!=sscanf(args,"%f,%f,%f",&v.x,&v.y,&v.z))	{ InvalidSyntax(); return; }
		if (v.x<min.x || v.y<min.y || v.z<min.z)		{ InvalidSyntax(); return; }
		if (v.x>max.x || v.y>max.y || v.z>max.z)		{ InvalidSyntax(); return; }
		value->set(v);
	}
	virtual void	Status	(TStatus& S)
	{	
		sprintf_s	(S,sizeof(S),"%f,%f,%f",value->x,value->y,value->z);
	}
	virtual void	Info	(TInfo& I)
	{	
		sprintf_s(I,sizeof(I),"vector3 in range [%f,%f,%f]-[%f,%f,%f]",min.x,min.y,min.z,max.x,max.y,max.z);
	}
};

class ENGINE_API	CCC_Integer : public IConsole_Command
{
protected	:
	int*			value;
	int				min,max;
public		:
	  const int GetValue	() const {return *value;};
	  const int GetMin		() const {return min;};
	  const int GetMax		() const {return max;};

	CCC_Integer(LPCSTR N, int* V, int _min=0, int _max=999) :
	  IConsole_Command(N),
	  value(V),
	  min(_min),
	  max(_max)
	{};

	virtual void	Execute	(LPCSTR args)
	{
		int v = atoi(args);
		if (v<min || v>max) InvalidSyntax();
		else	*value = v;
	}
	virtual void	Status	(TStatus& S)
	{	
		itoa(*value,S,10);
	}
	virtual void	Info	(TInfo& I)
	{	
		sprintf_s(I,sizeof(I),"integer value in range [%d,%d]",min,max);
	}
};

class ENGINE_API	CCC_String : public IConsole_Command
{
protected:
	LPSTR			value;
	int				size;
public:
	CCC_String(LPCSTR N, LPSTR V, int _size=2) :
		IConsole_Command(N),
		value	(V),
		size	(_size)
	{
		bLowerCaseArgs	=	FALSE;
		R_ASSERT(V);
		R_ASSERT(size>1);
	};

	virtual void	Execute	(LPCSTR args)
	{
		strncpy	(value,args,size-1);
	}
	virtual void	Status	(TStatus& S)
	{	
		strcpy_s	(S,value);
	}
	virtual void	Info	(TInfo& I)
	{	
		sprintf_s(I,sizeof(I),"string with up to %d characters",size);
	}
};

class ENGINE_API CCC_UserParam :public IConsole_Command
{
protected:
	xr_string			value;
public:
	CCC_UserParam(LPCSTR N, xr_string V) :IConsole_Command(N), value(V)
	{
		bEmptyArgsHandled = true;
	}
	xr_string GetValue() const { return value; }
	void SetValue(LPCSTR inval) { value = xr_strdup(inval); }

	void Save(IWriter* F) override
	{
		if (value.length() > 0)
			F->w_printf("%s %s\r\n", cName, value.c_str());
	};

	void Execute(LPCSTR args) override
	{
		if (xr_strlen(args) == 0)
		{
			TStatus S;
			Status(S);
			Log(S);
		}
		else
			SetValue(args);
	};

	void Status(TStatus& S) override
	{
		string256 buf;
		sprintf_s(buf, "Value of user-defined param [%s] is [%s]", cName, value.c_str());
		strcpy_s(S, buf);

	};

	void Info(TInfo& I) override
	{
		sprintf_s(I, "%s any_value_string --- user-defined param for store any value in string ", cName);
	};
};

class ENGINE_API CCC_LoadCFG : public IConsole_Command
{
public:
	virtual bool	allow			(LPCSTR cmd)	{return true;};
					CCC_LoadCFG		(LPCSTR N);
	virtual void	Execute			(LPCSTR args);
};

class ENGINE_API CCC_LoadCFG_custom : public CCC_LoadCFG
{
	string64		m_cmd;
public:
					CCC_LoadCFG_custom(LPCSTR cmd);
	virtual bool	allow			(LPCSTR cmd);
};

class ENGINE_API	CCC_SharedString : public IConsole_Command
{
protected:
	shared_str *value;
public:
	CCC_SharedString(LPCSTR N, shared_str *V) :
		IConsole_Command(N)
	{
		bLowerCaseArgs = true;
		value = V;
	};

	void	Execute(LPCSTR args) override
	{
		*value = args;
	}

	void	Status(TStatus& S) override
	{
		if (value->size()>0)
			strcpy_s(S, (*value).c_str());
		else
			strcpy_s(S, "default");
	}

	void	Info(TInfo& I) override
	{
		sprintf_s(I, sizeof(I), "section name of hud custom cursor");
	}
};
