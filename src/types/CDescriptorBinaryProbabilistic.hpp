#ifndef CDESCRIPTORBINARYPROBABILISTIC_HPP
#define CDESCRIPTORBINARYPROBABILISTIC_HPP

#include <bitset>
#include <Eigen/Core>



template< uint32_t uDescriptorSizeBits = 256, typename tPrecision = double >
struct CDescriptorBinaryProbabilistic
{
    //ds readability
    using CDescriptorVector    = std::bitset< uDescriptorSizeBits >;
    using CBitStatisticsVector = Eigen::Matrix< tPrecision, uDescriptorSizeBits, 1 >;

    CDescriptorBinaryProbabilistic( const uint64_t& p_uID,
                                    const CDescriptorVector& p_vecDescriptorValue,
                                    const CBitStatisticsVector& p_vecBitProbabilities,
                                    const CBitStatisticsVector& p_vecBitPermanences ): uID( p_uID ),
                                                                                       vecValues( p_vecDescriptorValue ),
                                                                                       vecBitProbabilities( p_vecBitProbabilities ),
                                                                                       vecBitPermanences( p_vecBitPermanences )
    {
        //ds nothing to do
    }

    ~CDescriptorBinaryProbabilistic( )
    {
        //ds nothing to do
    }

    //ds descriptor ID
    const uint64_t uID;

    //ds descriptor data string vector
    const CDescriptorVector vecValues;

    //ds statistical data: bit probabilities
    const CBitStatisticsVector vecBitProbabilities;

    //ds statistical data: bit permanences (bit volatity info, maximum number of appearances where the bit was stable)
    const CBitStatisticsVector vecBitPermanences;

};

#endif //CDESCRIPTORBINARYPROBABILISTIC_HPP
