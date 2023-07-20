//=============================================================================
//  Filename:   UIListWnd_inline.h
//	Created by Roman E. Marchenko, vortex@gsc-game.kiev.ua
//	Copyright 2004. GSC Game World
//	---------------------------------------------------------------------------
//  ���������� ������������ ������� ���������
//=============================================================================

template <class Element>
bool CUIListWnd::AddItem(const char*  str, const float shift, void* pData,
						 int value, int insertBeforeIdx)
{
	//������� ����� ������� � �������� ��� � ������
	Element* pItem = NULL;
	pItem = xr_new<Element>();

	VERIFY(pItem);

	pItem->Init(str, shift, m_bVertFlip?GetHeight()-GetItemsCount()* m_iItemHeight-m_iItemHeight: GetItemsCount()* m_iItemHeight,
		m_iItemWidth, m_iItemHeight);

	pItem->SetData(pData);
	pItem->SetValue(value);
	pItem->SetTextColor(m_dwFontColor);

	return AddItem<Element>(pItem, insertBeforeIdx);
}


template <class Element>
bool CUIListWnd::AddItem(Element* pItem, int insertBeforeIdx)
{	
	AttachChild(pItem);
	float itemsHeight=0;
	float separatorsHeight=0;
	std::for_each(m_ItemList.begin(), m_ItemList.end(), [&](CUIListItem* item)
	{
		if (item->m_bSeparator)
			separatorsHeight += item->GetHeight();
		else
			itemsHeight += m_iItemHeight;
	});
	float totalHeight = itemsHeight + separatorsHeight;
	float itemHeight = pItem->m_bSeparator ? pItem->GetHeight() : m_iItemHeight;
	pItem->Init(pItem->GetWndRect().left, m_bVertFlip?
											GetHeight()- totalHeight - itemHeight :
											totalHeight,
		m_iItemWidth, itemHeight);


	//���������� � ����� ��� ������ ������
	if(-1 == insertBeforeIdx)
	{
		m_ItemList.push_back(pItem);
		pItem->SetIndex(m_ItemList.size()-1);
	}
	else
	{
		//�������� �������� �������� ��� ���������� ���������
		if (!m_ItemList.empty())
			R_ASSERT(static_cast<u32>(insertBeforeIdx) <= m_ItemList.size());

		LIST_ITEM_LIST_it it2 = m_ItemList.begin();
		std::advance(it2, insertBeforeIdx);
		for(LIST_ITEM_LIST_it it = it2; m_ItemList.end() != it; ++it)
		{
			(*it)->SetIndex((*it)->GetIndex()+1);
		}
		m_ItemList.insert(it2, pItem);
		pItem->SetIndex(insertBeforeIdx);
	}

	UpdateList();

	//�������� ������ ���������
	m_ScrollBar->SetRange(0,s16(m_ItemList.size()-1));
	m_ScrollBar->SetPageSize(s16(
		(u32)m_iRowNum<m_ItemList.size()?m_iRowNum:m_ItemList.size()));
	m_ScrollBar->SetScrollPos(s16(m_iFirstShownIndex));
//	m_ScrollBar.Refresh();

	UpdateScrollBar();
	return true;
}