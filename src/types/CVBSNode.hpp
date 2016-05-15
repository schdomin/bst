#ifndef CVBSNODE_HPP
#define CVBSNODE_HPP

#include <vector>
#include <assert.h>
#include <cmath>

#include "CDescriptorBinaryProbabilistic.hpp"



template< uint64_t uMaximumDepth = 50, uint32_t uDescriptorSizeBits = 256, typename tPrecision = double >
class CVBSNode
{

    //ds readability
    using CNode                = CVBSNode< uMaximumDepth, uDescriptorSizeBits, tPrecision >;
    using CDescriptorValues    = std::bitset< uDescriptorSizeBits >;
    using CDescriptor          = CDescriptorBinaryProbabilistic< uDescriptorSizeBits >;
    using CBitStatisticsVector = Eigen::Matrix< tPrecision, uDescriptorSizeBits, 1 >;

//ds ctor/dtor
public:

    //ds access only through this constructor: no mask provided
    CVBSNode( const std::vector< CDescriptor >& p_vecDescriptors ): CNode( 0, p_vecDescriptors, _getMaskClean( ) )
    {
        //ds nothing to do
    }

    //ds access only through this constructor: mask provided
    CVBSNode( const std::vector< CDescriptor >& p_vecDescriptors, CDescriptorValues p_vecBitMask ): CNode( 0, p_vecDescriptors, p_vecBitMask )
    {
        //ds nothing to do
    }

    const bool spawnLeafs( )
    {
        //ds buffer number of descriptors
        const uint64_t uNumberOfDescriptors = vecDescriptors.size( );

        //ds if there are at least 2 descriptors (minimal split)
        if( 1 < uNumberOfDescriptors )
        {
            assert( !bHasLeaves );

            //ds affirm initial situation
            uIndexSplitBit = -1;
            uOnesTotal     = 0;
            dPartitioning  = 1.0;
            tPrecision dVarianceMaximum = 0.0;

            //ds variance computation statistics
            CBitStatisticsVector vecAccumulatedProbabilities( CBitStatisticsVector::Zero( ) );

            //ds for all descriptors in this node
            for( const CDescriptor& cDescriptor: vecDescriptors )
            {
                //ds reduce accumulated probabilities
                vecAccumulatedProbabilities += cDescriptor.vecBitProbabilities;
            }

            //ds get average
            const CBitStatisticsVector vecMean( vecAccumulatedProbabilities/uNumberOfDescriptors );
            std::vector< std::pair< uint32_t, tPrecision > > vecVariance( uDescriptorSizeBits );

            //ds compute variance
            for( uint32_t uIndexBit = 0; uIndexBit < uDescriptorSizeBits; ++uIndexBit )
            {
                //ds if this index is available in the mask
                if( matMask[uIndexBit] )
                {
                    //ds buffers
                    double dVarianceCurrent = 0.0;

                    //ds for all descriptors in this node
                    for( const CDescriptor& cDescriptor: vecDescriptors )
                    {
                        //ds update variance value
                        const double dDelta = cDescriptor.vecBitProbabilities[uIndexBit]-vecMean[uIndexBit];
                        dVarianceCurrent += dDelta*dDelta;
                    }

                    //ds average
                    dVarianceCurrent /= uNumberOfDescriptors;

                    //ds check if better
                    if( dVarianceMaximum < dVarianceCurrent )
                    {
                        dVarianceMaximum = dVarianceCurrent;
                        uIndexSplitBit   = uIndexBit;
                    }
                }
            }

            //ds if best was found - we can spawn leaves
            if( -1 != uIndexSplitBit && uMaximumDepth > uDepth )
            {
                //ds compute distance for this index (0.0 is perfect)
                dPartitioning = std::fabs( 0.5-_getOnesFraction( uIndexSplitBit, vecDescriptors, uOnesTotal ) );

                //ds check if we have enough data to split (NOT REQUIRED IF DEPTH IS SET ACCORDINGLY)
                if( 0 < uOnesTotal && 0.5 > dPartitioning )
                {
                    //ds enabled
                    bHasLeaves = true;

                    //ds get a mask copy
                    CDescriptorValues vecMask( matMask );

                    //ds update mask for leafs
                    vecMask[uIndexSplitBit] = 0;

                    //ds first we have to split the descriptors by the found index - preallocate vectors since we know how many ones we have
                    std::vector< CDescriptor > vecDescriptorsLeafOnes;
                    vecDescriptorsLeafOnes.reserve( uOnesTotal );
                    std::vector< CDescriptor > vecDescriptorsLeafZeros;
                    vecDescriptorsLeafZeros.reserve( uNumberOfDescriptors-uOnesTotal );

                    //ds loop over all descriptors and assing them to the new vectors
                    for( const CDescriptor& cDescriptor: vecDescriptors )
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

//ds internal ctors
private:

    //ds only internally called: without split order
    CVBSNode( const uint64_t& p_uDepth,
             const std::vector< CDescriptor >& p_vecDescriptors,
             CDescriptorValues p_vecMask ): uDepth( p_uDepth ),
                                            vecDescriptors( p_vecDescriptors ),
                                            matMask( p_vecMask )
    {
        //ds call recursive leaf spawner
        spawnLeafs( );
    }

//ds public dtor
public:

    ~CVBSNode( )
    {
        //ds nothing to do (the leafs will be freed by the tree)
    }

//ds fields
public:

    //ds rep
    const uint64_t uDepth;
    const std::vector< CDescriptor > vecDescriptors;
    int32_t uIndexSplitBit = -1;
    uint64_t uOnesTotal    = 0;
    bool bHasLeaves        = false;
    tPrecision dPartitioning   = 1.0;
    const CDescriptorValues matMask;

    //ds peer: each node has two potential children
    CNode* pLeafOnes  = 0;
    CNode* pLeafZeros = 0;

//ds helpers
private:

    //ds helpers
    const tPrecision _getOnesFraction( const uint32_t& p_uIndexSplitBit, const std::vector< CDescriptor >& p_vecDescriptors, uint64_t& p_uOnesTotal ) const
    {
        assert( 0 < p_vecDescriptors.size( ) );

        //ds count
        uint64_t uNumberOfOneBits = 0;

        //ds just add the bits up (a one counts automatically as one)
        for( const CDescriptor& cDescriptor: p_vecDescriptors )
        {
            uNumberOfOneBits += cDescriptor.vecValues[p_uIndexSplitBit];
        }

        //ds set total
        p_uOnesTotal = uNumberOfOneBits;
        assert( p_uOnesTotal <= p_vecDescriptors.size( ) );

        //ds return ratio
        return ( static_cast< tPrecision >( uNumberOfOneBits )/p_vecDescriptors.size( ) );
    }

    //ds returns a bitset with all bits set to true
    CDescriptorValues _getMaskClean( ) const
    {
        CDescriptorValues vecMask;
        vecMask.set( );
        return vecMask;
    }

    //ds filters multiple descriptors
    void _filterDescriptorsExhaustive( )
    {
        //ds unique descriptors (already add the front one first -> must be unique)
        std::vector< CDescriptor > vecDescriptorsUNIQUE( 1, vecDescriptors.front( ) );

        //ds loop over current ones
        for( const CDescriptor& cDescriptor: vecDescriptors )
        {
            //ds check if matched
            bool bNotFound = true;

            //ds check uniques
            for( const CDescriptor& cDescriptorUNIQUE: vecDescriptorsUNIQUE )
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

//ds external helpers (used by tree)
public:

    //ds computes Hamming distance for bitset descriptors
    inline static const uint32_t getDistanceHAMMING( const CDescriptorValues& p_vecDescriptorQuery,
                                                     const CDescriptorValues& p_vecDescriptorReference )
    {
        //ds count set bits
        return ( p_vecDescriptorQuery ^ p_vecDescriptorReference ).count( );
    }

    //ds filters multiple descriptors
    inline static const std::vector< CDescriptor > getFilteredDescriptorsExhaustive( const std::vector< CDescriptor >& p_vecDescriptors )
    {
        //ds unique descriptors (already add the front one first -> must be unique)
        std::vector< CDescriptor > vecDescriptorsUNIQUE( 1, p_vecDescriptors.front( ) );

        //ds loop over current ones
        for( const CDescriptor& cDescriptor: p_vecDescriptors )
        {
            //ds check if matched
            bool bNotFound = true;

            //ds check uniques
            for( const CDescriptor& cDescriptorUNIQUE: vecDescriptorsUNIQUE )
            {
                //ds assuming key frame identity
                assert( cDescriptorUNIQUE.uIDKeyFrame == cDescriptor.uIDKeyFrame );

                //ds if the actual descriptor is identical - and the key frame ID as well
                if( 0 == CNode::getDistanceHAMMING( cDescriptorUNIQUE.vecValues, cDescriptor.vecValues ) )
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

//ds utility functions
public:

    //ds generate dummy descriptors
    template< uint64_t uNumberOfDescriptors = 10000 >
    inline static const std::shared_ptr< std::vector< CDescriptor > > getDescriptorsDummy( )
    {
        //ds allocate output vector
        std::vector< CDescriptor > vecDescriptors;

        //ds create random entries
        for( uint64_t uIDDescriptor = 0; uIDDescriptor < uNumberOfDescriptors; ++uIDDescriptor )
        {
            //ds set the descriptor vector
            CDescriptorValues vecDescriptor;

            //ds create descriptor
            vecDescriptors.push_back( CDescriptor( uIDDescriptor, vecDescriptor, CBitStatisticsVector::Zero( ), CBitStatisticsVector::Zero( ) ) );
        }

        //ds return with generated descriptors
        return std::make_shared< std::vector< CDescriptor > >( vecDescriptors );
    }

};

#endif //CVBSNODE_HPP
