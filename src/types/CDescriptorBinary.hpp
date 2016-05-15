#ifndef CDESCRIPTORBINARY_HPP
#define CDESCRIPTORBINARY_HPP

#include <bitset>



template< uint32_t uDescriptorSizeBits = 256 >
struct CDescriptorBinary
{
    //ds readability
    using CDescriptorVector = std::bitset< uDescriptorSizeBits >;

    CDescriptorBinary( const uint64_t& p_uID,
                       const CDescriptorVector& p_vecDescriptorValue ): uID( p_uID ),
                                                                        vecValues( p_vecDescriptorValue )
    {
        //ds nothing to do
    }

    ~CDescriptorBinary( )
    {
        //ds nothing to do
    }

    const uint64_t uID;
    const CDescriptorVector vecValues;

};

#endif //CDESCRIPTORBINARY_HPP
