#pragma once
#include <vector>

namespace agn
{
	/*template<typename _T>
	class Histogram
	{
	public:



	private:

		size_t bins;
		std::vector<_T> data;
	};*/

	template<typename _T>
	std::tuple<std::vector<_T>, std::vector<_T>, size_t> SortedVectorToHistogram(std::vector<_T>& _data, size_t _bin_count, _T _min, _T _max)
	{
		std::vector<_T> histogram;
		std::vector<_T> histogram_labels;
		histogram.resize(_bin_count + 1, 0);
		histogram_labels.resize(_bin_count + 1, 0);

		std::vector<_T> ranges;
		_T bin_size = (_max - _min) / (_bin_count - 1);
		_T bin_cutoff = _min;
		for (size_t index = 0; index < _bin_count; ++index, bin_cutoff += bin_size)
		{
			ranges.push_back(bin_cutoff);
		}
		size_t highest_bin = 0;
		for (auto element : _data)
		{
			/*if (element < _min || element > _max)
			{
				histogram[_bin_count]++;
			}*/
			auto bin = _bin_count;
			for (size_t bin_index = 0; bin_index < _bin_count; ++bin_index)
			{
				_T cutoff = ranges[bin_index];
				if (element <= cutoff)
				{
					bin = bin_index;
					break;
				}
			}
			histogram_labels[bin] = element;
			histogram[bin]++;
			if (histogram[bin] > highest_bin)
				highest_bin = histogram[bin];
		}
		return std::make_tuple(std::move(histogram), std::move(histogram_labels), highest_bin);
	}
}