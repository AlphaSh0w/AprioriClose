#pragma once
#include <vector>
#include <map>
#include <string>
#include "rapidcsv.h"

class Itemset
{
public:
	Itemset(std::vector<std::pair<size_t, std::string>> items, const std::vector<std::map<std::string, std::vector<size_t>>>& tids, const rapidcsv::Document& document);
	void CalculateMetrics();
	float GetSupport() const;
private:
	float support;
	std::vector<std::pair<size_t,std::string>> items;
	const std::vector<std::map<std::string, std::vector<size_t>>>& tids;
	const rapidcsv::Document& document;
};