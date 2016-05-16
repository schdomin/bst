#ifndef CDESCRIPTORBINARYPROBABILISTIC_HPP
#define CDESCRIPTORBINARYPROBABILISTIC_HPP

#include "CDescriptorBinary.hpp"
#include <Eigen/Core>



template< uint32_t uDescriptorSizeBits = 256, typename tPrecision = double >
class CDescriptorBinaryProbabilistic: public CDescriptorBinary< uDescriptorSizeBits, tPrecision >
{

//ds template exports
public:

    typedef std::bitset< uDescriptorSizeBits > CDescriptorValues;
    typedef Eigen::Matrix< tPrecision, uDescriptorSizeBits, 1 > CBitStatisticsVector;

//ds ctor/dtor
public:

    CDescriptorBinaryProbabilistic( const uint64_t& p_uID,
                                    const CDescriptorValues& p_vecDescriptorValue,
                                    const CBitStatisticsVector& p_vecBitProbabilities,
                                    const CBitStatisticsVector& p_vecBitPermanences ): CDescriptorBinary< uDescriptorSizeBits, tPrecision >( p_uID, p_vecDescriptorValue ),
                                                                                       vecBitProbabilities( p_vecBitProbabilities ),
                                                                                       vecBitPermanences( p_vecBitPermanences )
    {
        //ds nothing to do
    }

    ~CDescriptorBinaryProbabilistic( )
    {
        //ds nothing to do
    }

//ds attributes
public:

    //ds statistical data: bit probabilities
    const CBitStatisticsVector vecBitProbabilities;

    //ds statistical data: bit permanences (bit volatity info, maximum number of appearances where the bit was stable)
    const CBitStatisticsVector vecBitPermanences;

};

#endif //CDESCRIPTORBINARYPROBABILISTIC_HPP
