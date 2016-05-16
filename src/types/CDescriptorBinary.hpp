#ifndef CDESCRIPTORBINARY_HPP
#define CDESCRIPTORBINARY_HPP

#include <bitset>



template< uint32_t uDescriptorSizeBits = 256, typename tPrecision = double >
class CDescriptorBinary
{

//ds template exports
public:

    typedef std::bitset< uDescriptorSizeBits > CDescriptorValues;

//ds shared properties
public:

    //ds descriptor size is set identically for all instances in the current setup
    static const uint32_t uSizeBits = uDescriptorSizeBits;

//ds ctor/dtor
public:

    CDescriptorBinary( const uint64_t& p_uID,
                       const CDescriptorValues& p_vecDescriptorValue ): uID( p_uID ),
                                                                        vecValues( p_vecDescriptorValue )
    {
        //ds nothing to do
    }

    ~CDescriptorBinary( )
    {
        //ds nothing to do
    }

//ds attributes
public:

    //ds descriptor ID
    const uint64_t uID;

    //ds descriptor data string vector
    const CDescriptorValues vecValues;

};

#endif //CDESCRIPTORBINARY_HPP
