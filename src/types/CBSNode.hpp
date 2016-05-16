#ifndef CBSNODE_HPP
#define CBSNODE_HPP

#include <vector>
#include <assert.h>
#include <cmath>

#include "CDescriptorBinary.hpp"



template< typename tDescriptor, uint64_t uMaximumDepth = 50, typename tPrecision = double >
class CBSNode
{

    //ds readability
    using CNode = CBSNode< tDescriptor, uMaximumDepth, tPrecision >;

//ds template exports
public:

    typedef tDescriptor CDescriptor;
    typedef typename tDescriptor::CDescriptorValues CDescriptorValues;

//ds ctor/dtor
public:

    //ds access only through this constructor: no mask provided
    CBSNode( const std::vector< const tDescriptor* >& p_vecDescriptors ): CNode( 0, getFilteredDescriptorsExhaustive( p_vecDescriptors ), _getMaskClean( ) )
    {
        //ds nothing to do
    }

    //ds access only through this constructor: mask provided
    CBSNode( const std::vector< const tDescriptor* >& p_vecDescriptors, typename tDescriptor::CDescriptorValues p_vecBitMask ): CNode( 0, getFilteredDescriptorsExhaustive( p_vecDescriptors ), p_vecBitMask )
    {
        //ds nothing to do
    }

    //ds access only through this constructor: split order provided
    CBSNode( const std::vector< const tDescriptor* >& p_vecDescriptors, std::vector< uint32_t > p_vecSplitOrder ): CNode( 0, getFilteredDescriptorsExhaustive( p_vecDescriptors ), p_vecSplitOrder )
    {
        //ds nothing to do
    }

//ds access
public:

    //ds create leafs (external use intented)
    virtual const bool spawnLeafs( )
    {
        //ds if there are at least 2 descriptors (minimal split)
        if( 1 < vecDescriptors.size( ) )
        {
            assert( !bHasLeaves );

            //ds affirm initial situation
            uIndexSplitBit = -1;
            uOnesTotal     = 0;
            dPartitioning  = 1.0;

            //ds we have to find the split for this node - scan all index
            for( uint32_t uIndexBit = 0; uIndexBit < tDescriptor::uSizeBits; ++uIndexBit )
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
                    typename tDescriptor::CDescriptorValues vecMask( matMask );

                    //ds update mask for leafs
                    vecMask[uIndexSplitBit] = 0;

                    //ds first we have to split the descriptors by the found index - preallocate vectors since we know how many ones we have
                    std::vector< const tDescriptor* > vecDescriptorsLeafOnes;
                    vecDescriptorsLeafOnes.reserve( uOnesTotal );
                    std::vector< const tDescriptor* > vecDescriptorsLeafZeros;
                    vecDescriptorsLeafZeros.reserve( vecDescriptors.size( )-uOnesTotal );

                    //ds loop over all descriptors and assing them to the new vectors
                    for( const tDescriptor* pDescriptor: vecDescriptors )
                    {
                        //ds check if split bit is set
                        if( pDescriptor->vecValues[uIndexSplitBit] )
                        {
                            vecDescriptorsLeafOnes.push_back( pDescriptor );
                        }
                        else
                        {
                            vecDescriptorsLeafZeros.push_back( pDescriptor );
                        }
                    }

                    //ds if there are elements for leaves
                    assert( 0 < vecDescriptorsLeafOnes.size( ) );
                    pLeafOnes = new CNode( uDepth+1, vecDescriptorsLeafOnes, vecMask );

                    assert( 0 < vecDescriptorsLeafZeros.size( ) );
                    pLeafZeros = new CNode( uDepth+1, vecDescriptorsLeafZeros, vecMask );

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
            //ds not enough descriptors to split
            return false;
        }
    }

    //ds create leafs following the set split order
    virtual const bool spawnLeafs( std::vector< uint32_t > p_vecSplitOrder )
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
                    std::vector< const tDescriptor* > vecDescriptorsLeafOnes;
                    vecDescriptorsLeafOnes.reserve( uOnesTotal );
                    std::vector< const tDescriptor* > vecDescriptorsLeafZeros;
                    vecDescriptorsLeafZeros.reserve( vecDescriptors.size( )-uOnesTotal );

                    //ds loop over all descriptors and assing them to the new vectors
                    for( const tDescriptor* pDescriptor: vecDescriptors )
                    {
                        //ds check if split bit is set
                        if( pDescriptor->vecValues[uIndexSplitBit] )
                        {
                            vecDescriptorsLeafOnes.push_back( pDescriptor );
                        }
                        else
                        {
                            vecDescriptorsLeafZeros.push_back( pDescriptor );
                        }
                    }

                    //ds if there are elements for leaves
                    assert( 0 < vecDescriptorsLeafOnes.size( ) );
                    pLeafOnes = new CNode( uDepth+1, vecDescriptorsLeafOnes, p_vecSplitOrder );

                    assert( 0 < vecDescriptorsLeafZeros.size( ) );
                    pLeafZeros = new CNode( uDepth+1, vecDescriptorsLeafZeros, p_vecSplitOrder );

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
            //ds not enough descriptors to split
            return false;
        }
    }

//ds specific constructors
public:

    //ds only internally called: without split order
    CBSNode( const uint64_t& p_uDepth,
             const std::vector< const tDescriptor* >& p_vecDescriptors,
             typename tDescriptor::CDescriptorValues p_vecMask ): uDepth( p_uDepth ), vecDescriptors( p_vecDescriptors ), matMask( p_vecMask )
    {
        //ds call recursive leaf spawner
        spawnLeafs( );
    }

    //ds only internally called: with split order
    CBSNode( const uint64_t& p_uDepth,
             const std::vector< const tDescriptor* >& p_vecDescriptors,
             std::vector< uint32_t > p_vecSplitOrder ): uDepth( p_uDepth ), vecDescriptors( p_vecDescriptors )
    {
        //ds call recursive leaf spawner
        spawnLeafs( p_vecSplitOrder );
    }

//ds public dtor
public:

    virtual ~CBSNode( )
    {
        //ds nothing to do (the leafs will be freed by the tree)
    }

//ds fields
public:

    //ds rep
    const uint64_t uDepth;
    const std::vector< const tDescriptor* > vecDescriptors;
    int32_t uIndexSplitBit   = -1;
    uint64_t uOnesTotal      = 0;
    bool bHasLeaves          = false;
    tPrecision dPartitioning = 1.0;
    const typename tDescriptor::CDescriptorValues matMask;

    //ds peer: each node has two potential children
    const CNode* pLeafOnes  = 0;
    const CNode* pLeafZeros = 0;

//ds helpers
protected:

    //ds helpers
    const tPrecision _getOnesFraction( const uint32_t& p_uIndexSplitBit, const std::vector< const tDescriptor* >& p_vecDescriptors, uint64_t& p_uOnesTotal ) const
    {
        assert( 0 < p_vecDescriptors.size( ) );

        //ds count
        uint64_t uNumberOfOneBits = 0;

        //ds just add the bits up (a one counts automatically as one)
        for( const tDescriptor* pDescriptor: p_vecDescriptors )
        {
            uNumberOfOneBits += pDescriptor->vecValues[p_uIndexSplitBit];
        }

        //ds set total
        p_uOnesTotal = uNumberOfOneBits;
        assert( p_uOnesTotal <= p_vecDescriptors.size( ) );

        //ds return ratio
        return ( static_cast< tPrecision >( uNumberOfOneBits )/p_vecDescriptors.size( ) );
    }

    //ds returns a bitset with all bits set to true
    typename tDescriptor::CDescriptorValues _getMaskClean( ) const
    {
        typename tDescriptor::CDescriptorValues vecMask;
        vecMask.set( );
        return vecMask;
    }

//ds external helpers (used by tree)
public:

    //ds computes Hamming distance for bitset descriptors (not to be implemented by subclasses)
    static const uint32_t getDistanceHAMMING( const typename tDescriptor::CDescriptorValues& p_vecDescriptorQuery,
                                              const typename tDescriptor::CDescriptorValues& p_vecDescriptorReference )
    {
        //ds count set bits
        return ( p_vecDescriptorQuery ^ p_vecDescriptorReference ).count( );
    }

    //ds filters multiple descriptors
    static const std::vector< const tDescriptor* > getFilteredDescriptorsExhaustive( const std::vector< const tDescriptor* >& p_vecDescriptors )
    {
        //ds unique descriptors (already add the front one first -> must be unique)
        std::vector< const tDescriptor* > vecDescriptorsUNIQUE( 1, p_vecDescriptors.front( ) );

        //ds loop over current ones
        for( const tDescriptor* pDescriptor: p_vecDescriptors )
        {
            //ds check if matched
            bool bNotFound = true;

            //ds check uniques
            for( const tDescriptor* pDescriptorUNIQUE: vecDescriptorsUNIQUE )
            {
                //ds if the actual descriptor is identical - and the key frame ID as well
                if( 0 == CNode::getDistanceHAMMING( pDescriptorUNIQUE->vecValues, pDescriptor->vecValues ) )
                {
                    //ds already added to the unique vector - no further adding required
                    bNotFound = false;
                    break;
                }
            }

            //ds check if we failed to match the descriptor against the unique ones
            if( bNotFound )
            {
                //ds new unique found
                vecDescriptorsUNIQUE.push_back( pDescriptor );
            }
        }

        assert( 0 < vecDescriptorsUNIQUE.size( ) );
        assert( vecDescriptorsUNIQUE.size( ) <= p_vecDescriptors.size( ) );

        //ds exchange internal version against unqiue
        return vecDescriptorsUNIQUE;
    }

};

#endif //CBSNODE_HPP
