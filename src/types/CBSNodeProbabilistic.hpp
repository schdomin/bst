#ifndef CBSNODEPROBABILISTIC_HPP
#define CBSNODEPROBABILISTIC_HPP

#include "CBSNode.hpp"                        //ds node base class
#include "CDescriptorBinaryProbabilistic.hpp" //ds custom descriptor



//ds Node template specialization for descriptors of type: CDescriptorBinaryProbabilistic
template< typename tDescriptor, uint64_t uMaximumDepth = 50, typename tPrecision = double >
class CBSNodeProbabilistic: public CBSNode< CDescriptorBinaryProbabilistic< >, uMaximumDepth, tPrecision >
{
    //ds readability
    using CNode = CBSNode< CDescriptorBinaryProbabilistic< >, uMaximumDepth, tPrecision >;

//ds template exports
public:

    typedef CDescriptorBinaryProbabilistic< > CDescriptor;
    typedef typename CDescriptorBinaryProbabilistic< >::CDescriptorValues CDescriptorValues;
    typedef typename CDescriptorBinaryProbabilistic< >::CBitStatisticsVector CBitStatisticsVector;

//ds ctor/dtor
public:

    //ds access only through this constructor: no mask provided
    CBSNodeProbabilistic( const std::vector< const CDescriptor* >& p_vecDescriptors ): CNode( p_vecDescriptors )
    {
        //ds nothing to do
    }

    //ds access only through this constructor: mask provided
    CBSNodeProbabilistic( const std::vector< const CDescriptor* >& p_vecDescriptors, CDescriptorValues p_vecBitMask ): CNode( p_vecDescriptors, p_vecBitMask )
    {
        //ds nothing to do
    }

//ds access
public:

    //ds implementing the new spawn leafs function to handle the modified descriptors
    virtual const bool spawnLeafs( )
    {
        //ds buffer number of descriptors
        const uint64_t uNumberOfDescriptors = this->vecDescriptors.size( );

        //ds if there are at least 2 descriptors (minimal split)
        if( 1 < uNumberOfDescriptors )
        {
            assert( !bHasLeaves );

            //ds affirm initial situation
            this->uIndexSplitBit = -1;
            this->uOnesTotal     = 0;
            this->dPartitioning  = 1.0;
            tPrecision dVarianceMaximum = 0.0;

            //ds variance computation statistics
            CBitStatisticsVector vecAccumulatedProbabilities( CBitStatisticsVector::Zero( ) );

            //ds for all descriptors in this node (use base class descriptor)
            for( const CDescriptor* pDescriptor: this->vecDescriptors )
            {
                //ds reduce accumulated probabilities (cast to the active descriptor)
                vecAccumulatedProbabilities += pDescriptor->vecBitProbabilities;
            }

            //ds get average
            const CBitStatisticsVector vecMean( vecAccumulatedProbabilities/uNumberOfDescriptors );
            std::vector< std::pair< uint32_t, tPrecision > > vecVariance( CDescriptor::uSizeBits );

            //ds compute variance
            for( uint32_t uIndexBit = 0; uIndexBit < CDescriptor::uSizeBits; ++uIndexBit )
            {
                //ds if this index is available in the mask
                if( this->matMask[uIndexBit] )
                {
                    //ds buffers
                    double dVarianceCurrent = 0.0;

                    //ds for all descriptors in this node
                    for( const CDescriptor* pDescriptor: this->vecDescriptors )
                    {
                        //ds update variance value
                        const double dDelta = pDescriptor->vecBitProbabilities[uIndexBit]-vecMean[uIndexBit];
                        dVarianceCurrent += dDelta*dDelta;
                    }

                    //ds average
                    dVarianceCurrent /= uNumberOfDescriptors;

                    //ds check if better
                    if( dVarianceMaximum < dVarianceCurrent )
                    {
                        dVarianceMaximum     = dVarianceCurrent;
                        this->uIndexSplitBit = uIndexBit;
                    }
                }
            }

            //ds if best was found - we can spawn leaves
            if( -1 != this->uIndexSplitBit && uMaximumDepth > this->uDepth )
            {
                //ds compute distance for this index (0.0 is perfect)
                this->dPartitioning = std::fabs( 0.5-this->_getOnesFraction( this->uIndexSplitBit, this->vecDescriptors, this->uOnesTotal ) );

                //ds check if we have enough data to split (NOT REQUIRED IF DEPTH IS SET ACCORDINGLY)
                if( 0 < this->uOnesTotal && 0.5 > this->dPartitioning )
                {
                    //ds enabled
                    this->bHasLeaves = true;

                    //ds get a mask copy
                    CDescriptorValues vecMask( this->matMask );

                    //ds update mask for leafs
                    vecMask[this->uIndexSplitBit] = 0;

                    //ds first we have to split the descriptors by the found index - preallocate vectors since we know how many ones we have
                    std::vector< const CDescriptor* > vecDescriptorsLeafOnes;
                    vecDescriptorsLeafOnes.reserve( this->uOnesTotal );
                    std::vector< const CDescriptor* > vecDescriptorsLeafZeros;
                    vecDescriptorsLeafZeros.reserve( uNumberOfDescriptors-this->uOnesTotal );

                    //ds loop over all descriptors and assing them to the new vectors
                    for( const CDescriptor* pDescriptor: this->vecDescriptors )
                    {
                        //ds check if split bit is set
                        if( pDescriptor->vecValues[this->uIndexSplitBit] )
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
                    this->pLeafOnes = new CNode( this->uDepth+1, vecDescriptorsLeafOnes, vecMask );

                    assert( 0 < vecDescriptorsLeafZeros.size( ) );
                    this->pLeafZeros = new CNode( this->uDepth+1, vecDescriptorsLeafZeros, vecMask );

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

//ds public dtor
public:

    virtual ~CBSNodeProbabilistic( )
    {
        //ds nothing to do (the leafs will be freed by the tree)
    }

};

#endif //CBSNODEPROBABILISTIC_HPP
