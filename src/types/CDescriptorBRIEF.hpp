#ifndef CDESCRIPTORBRIEF_HPP
#define CDESCRIPTORBRIEF_HPP

#include <bitset>



template< uint32_t uDescriptorSizeBits = 256 >
struct CDescriptorBRIEF
{
    //ds readability
    using CDescriptorVector = std::bitset< uDescriptorSizeBits >;

    CDescriptorBRIEF( const uint64_t& p_uID,
                      const CDescriptorVector& p_vecDescriptorValue ): uID( p_uID ),
                                                                       vecValue( p_vecDescriptorValue )
    {
        //ds nothing to do
    }

    ~CDescriptorBRIEF( )
    {
        //ds nothing to do
    }

    const uint64_t uID;
    const CDescriptorVector vecValue;

};

#endif //CDESCRIPTORBRIEF_HPP
