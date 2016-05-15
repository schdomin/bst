#ifndef CBSNODE_HPP
#define CBSNODE_HPP

#include <vector>
#include <assert.h>
#include <cmath>

#include "CDescriptorBinary.hpp"



template< uint64_t uMaximumDepth = 50, uint32_t uDescriptorSizeBits = 256 >
class CBSNode
{

    //ds readability
    using CDescriptorVector = std::bitset< uDescriptorSizeBits >;

//ds ctor/dtor
public:

    //ds access only through this constructor: no mask provided
    CBSNode( const std::vector< CDescriptorBinary< uDescriptorSizeBits > >& p_vecDescriptors ): CBSNode< uMaximumDepth, uDescriptorSizeBits >( 0, p_vecDescriptors, _getMaskClean( ) )
    {
        //ds wrapped
    }

    //ds access only through this constructor: mask provided
    CBSNode( const std::vector< CDescriptorBinary< uDescriptorSizeBits > >& p_vecDescriptors, CDescriptorVector p_vecBitMask ): CBSNode< uMaximumDepth, uDescriptorSizeBits >( 0, p_vecDescriptors, p_vecBitMask )
    {
        //ds wrapped
    }

    //ds access only through this constructor: split order provided
    CBSNode( const std::vector< CDescriptorBinary< uDescriptorSizeBits > >& p_vecDescriptors,
            std::vector< uint32_t > p_vecSplitOrder ): CBSNode< uMaximumDepth, uDescriptorSizeBits >( 0, p_vecDescriptors, p_vecSplitOrder )
    {
        //ds wrapped
    }

    //ds create leafs (external use intented)
    bool spawnLeafs( )
    {
        //ds filter descriptors before leafing
        //_filterDescriptorsExhaustive( );

        //ds if there are at least 2 descriptors (minimal split)
        if( 1 < vecDescriptors.size( ) )
        {
            assert( !bHasLeaves );

            //ds affirm initial situation
            uIndexSplitBit = -1;
            uOnesTotal     = 0;
            dPartitioning  = 1.0;

            //ds we have to find the split for this node - scan all index
            for( uint32_t uIndexBit = 0; uIndexBit < uDescriptorSizeBits; ++uIndexBit )
            {
                //ds if this index is available in the mask
                if( matMask[uIndexBit] )
                {
                    //ds temporary set bit count
                    uint64_t uNumberOfSetBits = 0;

                    //ds compute distance for this index (0.0 is perfect)
                    const double fPartitioningCurrent = std::fabs( 0.5-_getOnesFraction( uIndexBit, vecDescriptors, uNumberOfSetBits ) );

                    //ds if better
                    if( dPartitioning > fPartitioningCurrent )
                    {
                        dPartitioning  = fPartitioningCurrent;
                        uOnesTotal     = uNumberOfSetBits;
                        uIndexSplitBit = uIndexBit;

                        //ds finalize loop if maximum target is reached
                        if( 0.0 == dPartitioning )
                        {
                            break;
                        }
                    }
                }
            }

            //ds if best was found - we can spawn leaves
            if( -1 != uIndexSplitBit && uMaximumDepth > uDepth )
            {
                //ds check if we have enough data to split (NOT REQUIRED IF DEPTH IS SET ACCORDINGLY)
                if( 0 < uOnesTotal && 0.5 > dPartitioning )
                {
                    /*if( 5 > uDepth)
                    {
                        std::cerr << "depth: " << uDepth << " bit: " << uIndexSplitBit << std::endl;
                    }*/

                    //ds enabled
                    bHasLeaves = true;

                    //ds get a mask copy
                    CDescriptorVector vecMask( matMask );

                    //ds update mask for leafs
                    vecMask[uIndexSplitBit] = 0;

                    //ds first we have to split the descriptors by the found index - preallocate vectors since we know how many ones we have
                    std::vector< CDescriptorBinary< uDescriptorSizeBits > > vecDescriptorsLeafOnes;
                    vecDescriptorsLeafOnes.reserve( uOnesTotal );
                    std::vector< CDescriptorBinary< uDescriptorSizeBits > > vecDescriptorsLeafZeros;
                    vecDescriptorsLeafZeros.reserve( vecDescriptors.size( )-uOnesTotal );

                    //ds loop over all descriptors and assing them to the new vectors
                    for( const CDescriptorBinary< uDescriptorSizeBits >& cDescriptor: vecDescriptors )
                    {
                        //ds check if split bit is set
                        if( cDescriptor.vecValues[uIndexSplitBit] )
                        {
                            vecDescriptorsLeafOnes.push_back( cDescriptor );
                        }
                        else
                        {
                            vecDescriptorsLeafZeros.push_back( cDescriptor );
                        }
                    }

                    //ds if there are elements for leaves
                    assert( 0 < vecDescriptorsLeafOnes.size( ) );
                    pLeafOnes = new CBSNode< uMaximumDepth, uDescriptorSizeBits >( uDepth+1, vecDescriptorsLeafOnes, vecMask );

                    assert( 0 < vecDescriptorsLeafZeros.size( ) );
                    pLeafZeros = new CBSNode< uMaximumDepth, uDescriptorSizeBits >( uDepth+1, vecDescriptorsLeafZeros, vecMask );

                    //ds worked
                    return true;
                }
                else
                {
                    //ds split failed
                    return false;
                }
            }
            else
            {
                //ds split failed
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    //ds create leafs following the set split order
    bool spawnLeafs( std::vector< uint32_t > p_vecSplitOrder )
    {
        //ds if there are at least 2 descriptors (minimal split)
        if( 1 < vecDescriptors.size( ) )
        {
            assert( !bHasLeaves );

            //ds affirm initial situation
            uIndexSplitBit = -1;
            uOnesTotal     = 0;
            dPartitioning  = 1.0;

            //uint32_t uShift = 0;
            //uint32_t uBitOptimal = p_vecSplitOrder[uDepth];

            //ds try a selection of available bit splits
            for( uint32_t uDepthTrial = uDepth; uDepthTrial < 2*uDepth+1; ++uDepthTrial )
            {
                uint64_t uOnesTotalCurrent = 0;

                //ds compute distance for this index (0.0 is perfect)
                const double dPartitioningCurrent = std::fabs( 0.5-_getOnesFraction( p_vecSplitOrder[uDepthTrial], vecDescriptors, uOnesTotalCurrent ) );

                if( dPartitioning > dPartitioningCurrent )
                {
                    //ds buffer found bit
                    const uint32_t uSplitBitBest = p_vecSplitOrder[uDepthTrial];

                    //ds shift the last best index to the chosen depth in a later step
                    p_vecSplitOrder[uDepthTrial] = uIndexSplitBit;

                    dPartitioning  = dPartitioningCurrent;
                    uOnesTotal     = uOnesTotalCurrent;
                    uIndexSplitBit = uSplitBitBest;

                    //ds update the split order vector to the current bit
                    p_vecSplitOrder[uDepth] = uSplitBitBest;

                    //uShift = uDepthTrial-uDepth;

                    //ds finalize loop if maximum target is reached
                    if( 0.0 == dPartitioning )
                    {
                        break;
                    }
                }
            }

            //ds if best was found - we can spawn leaves
            if( -1 != uIndexSplitBit && uMaximumDepth > uDepth )
            {
                //ds check if we have enough data to split (NOT REQUIRED IF DEPTH IS SET ACCORDINGLY)
                if( 0 < uOnesTotal && 0.5 > dPartitioning )
                {
                    /*if( 5 > uDepth)
                    {
                        std::cerr << "depth: " << uDepth << " bit: " << uIndexSplitBit << " optimal: " << uBitOptimal <<  " shift: " << uShift << std::endl;
                    }*/

                    //ds enabled
                    bHasLeaves = true;

                    //ds first we have to split the descriptors by the found index - preallocate vectors since we know how many ones we have
                    std::vector< CDescriptorBinary< uDescriptorSizeBits > > vecDescriptorsLeafOnes;
                    vecDescriptorsLeafOnes.reserve( uOnesTotal );
                    std::vector< CDescriptorBinary< uDescriptorSizeBits > > vecDescriptorsLeafZeros;
                    vecDescriptorsLeafZeros.reserve( vecDescriptors.size( )-uOnesTotal );

                    //ds loop over all descriptors and assing them to the new vectors
                    for( const CDescriptorBinary< uDescriptorSizeBits >& cDescriptor: vecDescriptors )
                    {
                        //ds check if split bit is set
                        if( cDescriptor.vecData[uIndexSplitBit] )
                        {
                            vecDescriptorsLeafOnes.push_back( cDescriptor );
                        }
                        else
                        {
                            vecDescriptorsLeafZeros.push_back( cDescriptor );
                        }
                    }

                    //ds if there are elements for leaves
                    assert( 0 < vecDescriptorsLeafOnes.size( ) );
                    pLeafOnes = new CBSNode< uMaximumDepth, uDescriptorSizeBits >( uDepth+1, vecDescriptorsLeafOnes, p_vecSplitOrder );

                    assert( 0 < vecDescriptorsLeafZeros.size( ) );
                    pLeafZeros = new CBSNode< uMaximumDepth, uDescriptorSizeBits >( uDepth+1, vecDescriptorsLeafZeros, p_vecSplitOrder );

                    //ds worked
                    return true;
                }
                else
                {
                    //ds split failed
                    return false;
                }
            }
            else
            {
                //ds split failed
                return false;
            }
        }
        else
        {
            return false;
        }
    }

private:

    //ds only internally called
    CBSNode( const uint64_t& p_uDepth,
             const std::vector< CDescriptorBinary< uDescriptorSizeBits > >& p_vecDescriptors,
             CDescriptorVector p_vecMask ): uDepth( p_uDepth ), vecDescriptors( p_vecDescriptors ), matMask( p_vecMask )
    {
        //ds call recursive leaf spawner
        spawnLeafs( );
    }

    //ds only internally called
    CBSNode( const uint64_t& p_uDepth,
             const std::vector< CDescriptorBinary< uDescriptorSizeBits > >& p_vecDescriptors,
             std::vector< uint32_t > p_vecSplitOrder ): uDepth( p_uDepth ), vecDescriptors( p_vecDescriptors )
    {
        //ds call recursive leaf spawner
        spawnLeafs( p_vecSplitOrder );
    }

public:

    ~CBSNode( )
    {
        //ds nothing to do (the leafs will be freed manually)
    }

//ds fields
public:

    //ds rep
    const uint64_t uDepth;
    const std::vector< CDescriptorBinary< uDescriptorSizeBits > > vecDescriptors;
    int32_t uIndexSplitBit = -1;
    uint64_t uOnesTotal    = 0;
    bool bHasLeaves        = false;
    double dPartitioning   = 1.0;
    const CDescriptorVector matMask;

    //ds info (incremented in tree during search)
    //uint64_t uLinkedPoints = 0;

    //ds peer: each node has two potential children
    CBSNode* pLeafOnes  = 0;
    CBSNode* pLeafZeros = 0;

//ds helpers
private:

    //ds helpers
    const double _getOnesFraction( const uint32_t& p_uIndexSplitBit, const std::vector< CDescriptorBinary< uDescriptorSizeBits > >& p_vecDescriptors, uint64_t& p_uOnesTotal ) const
    {
        assert( 0 < p_vecDescriptors.size( ) );

        //ds count
        uint64_t uNumberOfOneBits = 0;

        //ds just add the bits up (a one counts automatically as one)
        for( const CDescriptorBinary< uDescriptorSizeBits >& cDescriptor: p_vecDescriptors )
        {
            uNumberOfOneBits += cDescriptor.vecValues[p_uIndexSplitBit];
        }

        //ds set total
        p_uOnesTotal = uNumberOfOneBits;
        assert( p_uOnesTotal <= p_vecDescriptors.size( ) );

        //ds return ratio
        return ( static_cast< float >( uNumberOfOneBits )/p_vecDescriptors.size( ) );
    }

    /*CDescriptor _getCopy( const CDescriptor& p_cMask ) const
    {
        CDescriptor cMaskCopy = new CDescriptorBRIEFElement[uDescriptorSize];
        for( uint32_t u = 0; u < uDescriptorSize; ++u )
        {
            cMaskCopy[u] = p_cMask[u];
        }
        return cMaskCopy;
    }*/

    //ds returns a bitset with all bits set to true
    CDescriptorVector _getMaskClean( ) const
    {
        CDescriptorVector vecMask;
        vecMask.set( );
        return vecMask;
    }

    //ds filters multiple descriptors
    void _filterDescriptorsExhaustive( )
    {
        //ds unique descriptors (already add the front one first -> must be unique)
        std::vector< CDescriptorBinary< uDescriptorSizeBits > > vecDescriptorsUNIQUE( 1, vecDescriptors.front( ) );

        //ds loop over current ones
        for( const CDescriptorBinary< uDescriptorSizeBits >& cDescriptor: vecDescriptors )
        {
            //ds check if matched
            bool bNotFound = true;

            //ds check uniques
            for( const CDescriptorBinary< uDescriptorSizeBits >& cDescriptorUNIQUE: vecDescriptorsUNIQUE )
            {
                //ds if the actual descriptor is identical - and the key frame ID as well
                if( ( 0 == getDistanceHAMMING( cDescriptorUNIQUE.vecData, cDescriptor.vecData ) ) &&
                    ( cDescriptorUNIQUE.uIDKeyFrame == cDescriptor.uIDKeyFrame )                  )
                {
                    //ds already added to the unique vector - no further adding required
                    bNotFound = false;
                    break;
                }
            }

            //ds check if we failed to match the descriptor against the unique ones
            if( bNotFound )
            {
                vecDescriptorsUNIQUE.push_back( cDescriptor );
            }
        }

        assert( 0 < vecDescriptorsUNIQUE.size( ) );
        assert( vecDescriptorsUNIQUE.size( ) <= vecDescriptors.size( ) );

        //ds exchange internal version against unqiue
        vecDescriptors.swap( vecDescriptorsUNIQUE );
    }

//ds format
public:

    //ds computes Hamming distance for bitset descriptors
    inline static const uint32_t getDistanceHAMMING( const CDescriptorVector& p_vecDescriptorQuery,
                                                     const CDescriptorVector& p_vecDescriptorReference )
    {
        //ds count set bits
        return ( p_vecDescriptorQuery ^ p_vecDescriptorReference ).count( );
    }

    //ds filters multiple descriptors
    inline static const std::vector< CDescriptorBinary< uDescriptorSizeBits > > getFilteredDescriptorsExhaustive( const std::vector< CDescriptorBinary< uDescriptorSizeBits > >& p_vecDescriptors )
    {
        //ds unique descriptors (already add the front one first -> must be unique)
        std::vector< CDescriptorBinary< uDescriptorSizeBits > > vecDescriptorsUNIQUE( 1, p_vecDescriptors.front( ) );

        //ds loop over current ones
        for( const CDescriptorBinary< uDescriptorSizeBits >& cDescriptor: p_vecDescriptors )
        {
            //ds check if matched
            bool bNotFound = true;

            //ds check uniques
            for( const CDescriptorBinary< uDescriptorSizeBits >& cDescriptorUNIQUE: vecDescriptorsUNIQUE )
            {
                //ds assuming key frame identity
                assert( cDescriptorUNIQUE.uIDKeyFrame == cDescriptor.uIDKeyFrame );

                //ds if the actual descriptor is identical - and the key frame ID as well
                if( 0 == CBSNode< uMaximumDepth, uDescriptorSizeBits >::getDistanceHAMMING( cDescriptorUNIQUE.vecValues, cDescriptor.vecValues ) )
                {
                    //ds already added to the unique vector - no further adding required
                    bNotFound = false;
                    break;
                }
            }

            //ds check if we failed to match the descriptor against the unique ones
            if( bNotFound )
            {
                vecDescriptorsUNIQUE.push_back( cDescriptor );
            }
        }

        assert( 0 < vecDescriptorsUNIQUE.size( ) );
        assert( vecDescriptorsUNIQUE.size( ) <= p_vecDescriptors.size( ) );

        //ds exchange internal version against unqiue
        return vecDescriptorsUNIQUE;
    }

    //ds generate random descriptors
    template< uint64_t uNumberOfDescriptors = 10000 >
    inline static const std::shared_ptr< std::vector< CDescriptorBinary< uDescriptorSizeBits > > > getRandomDescriptors( )
    {
        //ds allocate output vector
        std::vector< CDescriptorBinary< uDescriptorSizeBits > > vecDescriptors;

        //ds create random entries
        for( uint64_t uIDDescriptor = 0; uIDDescriptor < uNumberOfDescriptors; ++uIDDescriptor )
        {
            //ds set the descriptor vector
            std::bitset< uDescriptorSizeBits > vecDescriptor;

            //ds create descriptor
            vecDescriptors.push_back( CDescriptorBinary< uDescriptorSizeBits >( uIDDescriptor, vecDescriptor ) );
        }

        //ds return with generated descriptors
        return std::make_shared< std::vector< CDescriptorBinary< uDescriptorSizeBits > > >( vecDescriptors );
    }

};

#endif //CBSNODE_HPP
