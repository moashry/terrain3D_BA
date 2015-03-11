#ifndef __TUM3D__TILETREE_H__
#define __TUM3D__TILETREE_H__


class Tile;

struct TileTree
{
	Tile*	m_root;
};

struct TreesInfo
{
	uint	m_uiNumTrees[2];
	int		m_iTreeX0, m_iTreeY0, m_iTreeX1, m_iTreeY1;
};


#endif
