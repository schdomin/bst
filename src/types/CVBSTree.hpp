#ifndef CVBSTREE_HPP
#define CVBSTREE_HPP

#include <memory>

#include "CVBSNode.hpp"
#include "CBSTMatch.hpp"



template< uint32_t uMaximumDistanceHamming = 25, uint64_t uMaximumDepth = 50, uint32_t uDescriptorSizeBits = 256, typename tPrecision = double >
class CVBSTree
{

    //ds readability
    using CNode                = CVBSNode< uMaximumDepth, uDescriptorSizeBits, tPrecision >;
    using CDescriptorVector    = std::bitset< uDescriptorSizeBits >;
    using CDescriptor          = CDescriptorBinaryProbabilistic< uDescriptorSizeBits >;

//ds ctor/dtor
public:

    //ds construct tree upon allocation on filtered descriptors
    CVBSTree( const uint64_t& p_uID,
              const std::vector< CDescriptor >& p_vecDescriptors ): uID( p_uID ),
                                                                    m_pRoot( new CNode( CNode::getFilteredDescriptorsExhaustive( p_vecDescriptors ) ) )
    {
        assert( 0 != m_pRoot );
    }

    //ds construct tree upon allocation on filtered descriptors: with mask
    CVBSTree( const uint64_t& p_uID,
              const std::vector< CDescriptor >& p_vecDescriptors,
              CDescriptorVector p_vecBitMask ): uID( p_uID ),
                                                m_pRoot( new CNode( CNode::getFilteredDescriptorsExhaustive( p_vecDescriptors ), p_vecBitMask ) )
    {
        assert( 0 != m_pRoot );
    }

    //ds free all nodes in the tree
    ~CVBSTree( )
    {
        //ds erase all nodes
        displant( );
    }



//ds control fields
public:

    const uint64_t uID;

private:

    const CNode* m_pRoot;

//ds access
public:

    const uint64_t getNumberOfMatches( const std::shared_ptr< std::vector< CDescriptor > > p_vecDescriptorsQuery, const uint32_t& p_uMaximumDistanceHamming ) const
    {
        //ds match count
        uint64_t uNumberOfMatches = 0;

        //ds for each descriptor
        for( const CDescriptor& cDescriptorQuery: *p_vecDescriptorsQuery )
        {
            //ds traverse tree to find this descriptor
            const CNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( cDescriptorQuery.vecData[pNodeCurrent->uIndexSplitBit] )
                    {
                        pNodeCurrent = pNodeCurrent->pLeafOnes;
                    }
                    else
                    {
                        pNodeCurrent = pNodeCurrent->pLeafZeros;
                    }
                }
                else
                {
                    //ds check current descriptors in this node and exit
                    for( const CDescriptor& cDescriptorReference: pNodeCurrent->vecDescriptors )
                    {
                        if( p_uMaximumDistanceHamming > CNode::getDistanceHamming( cDescriptorQuery.vecData, cDescriptorReference.vecData ) )
                        {
                            ++uNumberOfMatches;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        return uNumberOfMatches;
    }

    const uint64_t getNumberOfMatchesLazyEvaluation( const std::shared_ptr< std::vector< CDescriptor > > p_vecDescriptorsQuery, const uint32_t& p_uMaximumDistanceHamming ) const
    {
        //ds match count
        uint64_t uNumberOfMatches = 0;

        //ds for each descriptor
        for( const CDescriptor& cDescriptorQuery: *p_vecDescriptorsQuery )
        {
            //ds traverse tree to find this descriptor
            const CNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( cDescriptorQuery.vecData[pNodeCurrent->uIndexSplitBit] )
                    {
                        pNodeCurrent = pNodeCurrent->pLeafOnes;
                    }
                    else
                    {
                        pNodeCurrent = pNodeCurrent->pLeafZeros;
                    }
                }
                else
                {
                    //ds check current descriptors in this node and exit
                    if( p_uMaximumDistanceHamming > CNode::getDistanceHamming( cDescriptorQuery.vecData, pNodeCurrent->vecDescriptors.front( ).vecData ) )
                    {
                        ++uNumberOfMatches;
                    }
                    break;
                }
            }
        }

        return uNumberOfMatches;
    }

    void setMatches1NN( const std::shared_ptr< std::vector< CDescriptor > > p_vecDescriptorsQUERY, const uint32_t& p_uIDTrain, std::vector< std::vector< CBSTMatch > >& p_vecMatches ) const
    {
        //ds for each descriptor
        for( const CDescriptor& cDescriptorQUERY: *p_vecDescriptorsQUERY )
        {
            //ds traverse tree to find this descriptor
            const CNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( cDescriptorQUERY.vecData[pNodeCurrent->uIndexSplitBit] )
                    {
                        pNodeCurrent = pNodeCurrent->pLeafOnes;
                    }
                    else
                    {
                        pNodeCurrent = pNodeCurrent->pLeafZeros;
                    }
                }
                else
                {
                    //ds check current descriptors in this node and exit
                    for( const CDescriptor& cDescriptorTRAIN: pNodeCurrent->vecDescriptors )
                    {
                        if( uMaximumDistanceHamming > CNode::getDistanceHamming( cDescriptorQUERY.vecData, cDescriptorTRAIN.vecData ) )
                        {
                            //++pNodeCurrent->uLinkedPoints;
                            p_vecMatches.push_back( std::vector< CBSTMatch >( 1, CBSTMatch( cDescriptorQUERY.uID, cDescriptorTRAIN.uID, p_uIDTrain, uMaximumDistanceHamming ) ) );
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    //ds direct matching function on this tree
    void match( const std::shared_ptr< std::vector< CDescriptor > > p_vecDescriptorsQUERY, std::vector< CBSTMatch >& p_vecMatches ) const
    {
        //ds for each descriptor
        for( const CDescriptor& cDescriptorQUERY: *p_vecDescriptorsQUERY )
        {
            //ds traverse tree to find this descriptor
            const CNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( cDescriptorQUERY.vecValues[pNodeCurrent->uIndexSplitBit] )
                    {
                        pNodeCurrent = pNodeCurrent->pLeafOnes;
                    }
                    else
                    {
                        pNodeCurrent = pNodeCurrent->pLeafZeros;
                    }
                }
                else
                {
                    //ds check current descriptors in this node and exit
                    for( const CDescriptor& cDescriptorTRAIN: pNodeCurrent->vecDescriptors )
                    {
                        if( uMaximumDistanceHamming > CNode::getDistanceHAMMING( cDescriptorQUERY.vecValues, cDescriptorTRAIN.vecValues ) )
                        {
                            //++pNodeCurrent->uLinkedPoints;
                            p_vecMatches.push_back( CBSTMatch( cDescriptorQUERY.uID, cDescriptorTRAIN.uID, uMaximumDistanceHamming ) );
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    //ds return matches directly
    const std::shared_ptr< std::vector< CBSTMatch > > getMatches( const std::shared_ptr< std::vector< CDescriptor > > p_vecDescriptorsQUERY ) const
    {
        //ds match vector to be filled
        std::vector< CBSTMatch > vecMatches;

        //ds for each QUERY descriptor
        for( const CDescriptor& cDescriptorQUERY: *p_vecDescriptorsQUERY )
        {
            //ds traverse tree to find this descriptor
            const CNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( cDescriptorQUERY.vecValues[pNodeCurrent->uIndexSplitBit] )
                    {
                        pNodeCurrent = pNodeCurrent->pLeafOnes;
                    }
                    else
                    {
                        pNodeCurrent = pNodeCurrent->pLeafZeros;
                    }
                }
                else
                {
                    //ds check current descriptors in this node and exit
                    for( const CDescriptor& cDescriptorTRAIN: pNodeCurrent->vecDescriptors )
                    {
                        if( uMaximumDistanceHamming > CNode::getDistanceHAMMING( cDescriptorQUERY.vecValues, cDescriptorTRAIN.vecValues ) )
                        {
                            vecMatches.push_back( CBSTMatch( cDescriptorQUERY.uID, cDescriptorTRAIN.uID, uMaximumDistanceHamming ) );
                            break;
                        }
                    }
                    break;
                }
            }
        }

        //ds return findings
        return std::make_shared< std::vector< CBSTMatch > >( vecMatches );
    }

    //ds grow the tree
    void plant( const std::vector< CDescriptor >& p_vecDescriptors )
    {
        //ds grow tree on root
        m_pRoot = new CNode( p_vecDescriptors );
    }

    //ds delete tree
    void displant( )
    {
        //ds nodes holder
        std::vector< const CNode* > vecNodes;

        //ds set vector
        _setNodesRecursive( m_pRoot, vecNodes );

        //ds free nodes
        for( const CNode* pNode: vecNodes )
        {
            delete pNode;
        }

        //ds free all nodes
        //std::printf( "(CBTree) deallocated nodes: %lu\n", vecNodes.size( ) );
        vecNodes.clear( );
    }

//ds helpers
private:

    void _setNodesRecursive( const CNode* p_pNode, std::vector< const CNode* >& p_vecNodes ) const
    {
        //ds must not be zero
        assert( 0 != p_pNode );

        //ds add the current node
        p_vecNodes.push_back( p_pNode );

        //ds check if there are leafs
        if( p_pNode->bHasLeaves )
        {
            //ds add leafs and so on
            _setNodesRecursive( p_pNode->pLeafOnes, p_vecNodes );
            _setNodesRecursive( p_pNode->pLeafZeros, p_vecNodes );
        }
    }

};

#endif //CVBSTREE_HPP
