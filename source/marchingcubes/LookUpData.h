#pragma once

#include <vector>

#include <glm/vec3.hpp>

class LookUpData 
{
public:
	LookUpData();
	~LookUpData();

public:
	static std::vector<int> m_caseToNumPolys;
	static std::vector<glm::ivec2> m_edgeToVertices;
	static std::vector<glm::ivec4> m_edgeConnectList;
};
