#include "ACloseAlgorithm.h"
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <stdio.h>
#include "Itemset.h"

ACloseAlgorithm::ACloseAlgorithm(const rapidcsv::Document& document)
	:
	document(document),
	k(1)
{
	//GenerateTIDs(document);
	GenerateTIDsMultiThreaded(document);
}

void ACloseAlgorithm::Go(const float minSup)
{
	const size_t rowCount = document.GetRowCount();
	do
	{	
		kItemsets.emplace_back(GenerateKItemsets(k));
		for (auto& item : kItemsets.back())
		{
			item.CalculateMetrics(tids, rowCount);
		}
		PruneUnfrequentItemsets(kItemsets.back(), minSup);
		++k;
	} while(!kItemsets.back().empty());

}

void ACloseAlgorithm::PrintItemsets() const
{
	for (size_t i = 0; i < kItemsets.size(); ++i)
	{
		std::cout << "\nSize of itemsets L(" << i+1 <<") : " << kItemsets[i].size() << "\n";
	}
}

void ACloseAlgorithm::GenerateTIDs(const rapidcsv::Document& document)
{
	//Allocate the necessarry memory to hold the TIDs
	tids.resize(document.GetColumnCount());
	//Go through the document and generate the TIDs.
	for (size_t iColumn = 0; iColumn < document.GetColumnCount(); iColumn++)
	{
		//Generate the column values for this column and fill its part in the TIDs.
		auto column = document.GetColumn<std::string>(iColumn);
		for (size_t iRow = 0; iRow < column.size(); ++iRow)
		{
			//Fill the TID of this cell
			tids[iColumn][column[iRow]].emplace_back(iRow);
		}
	}
}

void ACloseAlgorithm::GenerateTIDsMultiThreaded(const rapidcsv::Document& document)
{
	//Allocate the necessarry memory to hold the TIDs
	tids.resize(document.GetColumnCount());
	//Create a worker thread for each column
	std::vector<std::thread> workers;
	//Go through the document and generate the columns values and the TIDs.
	for (size_t iColumn = 0; iColumn < document.GetColumnCount(); ++iColumn)
	{
		const auto worker = [this](const rapidcsv::Document& document,const size_t iColumn) {
			this->GenerateColumnTID(document,iColumn);
		};
		workers.push_back(std::thread{worker, std::ref(document), iColumn});
	}
	//Wait for all workers to finish
	for (auto& worker : workers)
	{
		worker.join();
	}
}

void ACloseAlgorithm::GenerateColumnTID(const rapidcsv::Document& document ,const size_t iColumn)
{
	//Generate the column values for this column and fill its part in the TIDs.
	auto column = document.GetColumn<std::string>(iColumn);
	for (size_t iRow = 0; iRow < column.size(); ++iRow)
	{
		//Fill the TID of this cell
		tids[iColumn][column[iRow]].emplace_back(iRow);
	}
}

std::vector<Itemset> ACloseAlgorithm::GenerateKItemsets(size_t k)
{
	std::vector<Itemset> generatedItemsets;
	//Generate k items
	if (k == 1)
	{
		//Generate 1st itemsets
		for (size_t i = 0; i < tids.size(); ++i)
		{
			for (const auto& tid : tids[i])
			{
				generatedItemsets.emplace_back(std::vector{ std::pair{i,tid.first} });
			}
		}
	}
	else
	{
		//Generate k-itemsets by combining the k-1 itemsets.
		const auto& previousItemsets = kItemsets[k - 2];
		for (size_t i = 0; i < previousItemsets.size() - 1; ++i)
		{
			for (size_t j = i + 1; j < previousItemsets.size(); ++j)
			{
				//Check if they have k - 2 first items in common
				if (previousItemsets[i].HasFirstKInCommon(previousItemsets[j],k-2))
				{
					//Get their union
					Itemset itemsetUnion = previousItemsets[i] + previousItemsets[j];
					//Check if this combination of items is possible
					if (itemsetUnion.IsValid())
					{
						generatedItemsets.emplace_back(itemsetUnion);
					}
				}
			}
		}
	}
	return generatedItemsets;
}

void ACloseAlgorithm::PruneUnfrequentItemsets(std::vector<Itemset>& itemsets, const float minSup)
{
	//Move all unfrequent itemsets to the end of the vector.
	auto toErase = std::remove_if(itemsets.begin(), itemsets.end(), [minSup](const Itemset& itemset) {
		return itemset.GetSupport() < minSup;
	});
	//Erase the unfrequent itemsets.
	itemsets.erase(toErase, itemsets.end());
}

