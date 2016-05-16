
#ifndef CBSTREE_HPP
#define CBSTREE_HPP

#include <memory>

#include "CBSTMatch.hpp"
#include "CBSNode.hpp"
#include "CBSNodeProbabilistic.hpp" //ds custom node



template< typename tBSNode, uint32_t uMaximumDistanceHamming = 25 >
class CBSTree
{

    //ds readability
    typedef typename tBSNode::CDescriptor CDescriptor;

//ds ctor/dtor
public:

    //ds construct tree upon allocation on filtered descriptors
    CBSTree( const uint64_t& p_uID,
             const std::vector< const CDescriptor* >& p_vecDescriptors ): uID( p_uID ),
                                                                          m_pRoot( new tBSNode( p_vecDescriptors ) )
    {
        assert( 0 != m_pRoot );
    }

    //ds construct tree upon allocation on filtered descriptors: with bit mask
    CBSTree( const uint64_t& p_uID,
             const std::vector< const CDescriptor* >& p_vecDescriptors,
             typename tBSNode::CDescriptorValues p_vecBitMask ): uID( p_uID ),
                                                 m_pRoot( new tBSNode( p_vecDescriptors, p_vecBitMask ) )
    {
        assert( 0 != m_pRoot );
    }

    //ds construct tree with fixed split order
    CBSTree( const uint64_t& p_uID,
             const std::vector< const CDescriptor* >& p_vecDescriptors,
             std::vector< uint32_t > p_vecSplitOrder ): uID( p_uID ),
                                                        m_pRoot( new tBSNode( p_vecDescriptors, p_vecSplitOrder ) )
    {
        assert( 0 != m_pRoot );
    }

    //ds free all nodes in the tree
    ~CBSTree( )
    {
        //ds erase all nodes (and descriptors)
        _displant( );
    }

//ds control fields
public:

    //ds tree ID
    const uint64_t uID;

private:

    //ds root node
    const tBSNode* m_pRoot;

//ds access
public:

    const uint64_t getNumberOfMatches( const std::shared_ptr< std::vector< const CDescriptor* > > p_vecDescriptorsQuery, const uint32_t& p_uMaximumDistanceHamming ) const
    {
        //ds match count
        uint64_t uNumberOfMatches = 0;

        //ds for each descriptor
        for( const CDescriptor* pDescriptorQuery: *p_vecDescriptorsQuery )
        {
            //ds traverse tree to find this descriptor
            const tBSNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( pDescriptorQuery->vecValues[pNodeCurrent->uIndexSplitBit] )
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
                    for( const CDescriptor* pDescriptorReference: pNodeCurrent->vecDescriptors )
                    {
                        if( p_uMaximumDistanceHamming > tBSNode::getDistanceHamming( pDescriptorQuery->vecValues, pDescriptorReference->vecValues ) )
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

    const uint64_t getNumberOfMatchesLazyEvaluation( const std::shared_ptr< std::vector< const CDescriptor* > > p_vecDescriptorsQuery, const uint32_t& p_uMaximumDistanceHamming ) const
    {
        //ds match count
        uint64_t uNumberOfMatches = 0;

        //ds for each descriptor
        for( const CDescriptor* pDescriptorQuery: *p_vecDescriptorsQuery )
        {
            //ds traverse tree to find this descriptor
            const tBSNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( pDescriptorQuery->vecValues[pNodeCurrent->uIndexSplitBit] )
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
                    if( p_uMaximumDistanceHamming > tBSNode::getDistanceHamming( pDescriptorQuery->vecValues, pNodeCurrent->vecDescriptors.front( )->vecValues ) )
                    {
                        ++uNumberOfMatches;
                    }
                    break;
                }
            }
        }

        return uNumberOfMatches;
    }

    void setMatches1NN( const std::shared_ptr< std::vector< const CDescriptor* > > p_vecDescriptorsQUERY, const uint32_t& p_uIDTrain, std::vector< std::vector< CBSTMatch< > > >& p_vecMatches ) const
    {
        //ds for each descriptor
        for( const CDescriptor* pDescriptorQUERY: *p_vecDescriptorsQUERY )
        {
            //ds traverse tree to find this descriptor
            const tBSNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( pDescriptorQUERY->vecValues[pNodeCurrent->uIndexSplitBit] )
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
                    for( const CDescriptor* pDescriptorTRAIN: pNodeCurrent->vecDescriptors )
                    {
                        if( uMaximumDistanceHamming > tBSNode::getDistanceHamming( pDescriptorQUERY->vecValues, pDescriptorTRAIN->vecValues ) )
                        {
                            //++pNodeCurrent->uLinkedPoints;
                            p_vecMatches.push_back( std::vector< CBSTMatch< > >( 1, CBSTMatch< >( pDescriptorQUERY->uID, pDescriptorTRAIN->uID, p_uIDTrain, uMaximumDistanceHamming ) ) );
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    //ds direct matching function on this tree
    void match( const std::shared_ptr< const std::vector< const CDescriptor* > > p_vecDescriptorsQUERY, std::vector< CBSTMatch< > >& p_vecMatches ) const
    {
        //ds for each descriptor
        for( const CDescriptor* pDescriptorQUERY: *p_vecDescriptorsQUERY )
        {
            //ds traverse tree to find this descriptor
            const tBSNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( pDescriptorQUERY->vecValues[pNodeCurrent->uIndexSplitBit] )
                    {
                        pNodeCurrent = static_cast< const tBSNode* >( pNodeCurrent->pLeafOnes );
                    }
                    else
                    {
                        pNodeCurrent = static_cast< const tBSNode* >( pNodeCurrent->pLeafZeros );
                    }
                }
                else
                {
                    //ds check current descriptors in this node and exit
                    for( const CDescriptor* pDescriptorTRAIN: pNodeCurrent->vecDescriptors )
                    {
                        if( uMaximumDistanceHamming > tBSNode::getDistanceHAMMING( pDescriptorQUERY->vecValues, pDescriptorTRAIN->vecValues ) )
                        {
                            //++pNodeCurrent->uLinkedPoints;
                            p_vecMatches.push_back( CBSTMatch< >( pDescriptorQUERY->uID, pDescriptorTRAIN->uID, uMaximumDistanceHamming ) );
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    //ds return matches directly
    const std::shared_ptr< const std::vector< CBSTMatch< > > > getMatches( const std::shared_ptr< const std::vector< const CDescriptor* > > p_vecDescriptorsQUERY ) const
    {
        //ds match vector to be filled
        std::vector< CBSTMatch< > > vecMatches;

        //ds for each QUERY descriptor
        for( const CDescriptor* pDescriptorQUERY: *p_vecDescriptorsQUERY )
        {
            //ds traverse tree to find this descriptor
            const tBSNode* pNodeCurrent = m_pRoot;
            while( pNodeCurrent )
            {
                //ds if this node has leaves (is splittable)
                if( pNodeCurrent->bHasLeaves )
                {
                    //ds check the split bit and go deeper
                    if( pDescriptorQUERY->vecValues[pNodeCurrent->uIndexSplitBit] )
                    {
                        pNodeCurrent = static_cast< const tBSNode* >( pNodeCurrent->pLeafOnes );
                    }
                    else
                    {
                        pNodeCurrent = static_cast< const tBSNode* >( pNodeCurrent->pLeafZeros );
                    }
                }
                else
                {
                    //ds check current descriptors in this node and exit
                    for( const CDescriptor* pDescriptorTRAIN: pNodeCurrent->vecDescriptors )
                    {
                        if( uMaximumDistanceHamming > tBSNode::getDistanceHAMMING( pDescriptorQUERY->vecValues, pDescriptorTRAIN->vecValues ) )
                        {
                            vecMatches.push_back( CBSTMatch< >( pDescriptorQUERY->uID, pDescriptorTRAIN->uID, uMaximumDistanceHamming ) );
                            break;
                        }
                    }
                    break;
                }
            }
        }

        //ds return findings
        return std::make_shared< const std::vector< CBSTMatch< > > >( vecMatches );
    }

    //ds grow the tree
    void plant( const std::vector< const CDescriptor* >& p_vecDescriptors )
    {
        //ds grow tree on root
        m_pRoot = new tBSNode( p_vecDescriptors );
    }

//ds helpers
private:

    //ds delete tree
    void _displant( )
    {
        //ds nodes holder
        std::vector< const tBSNode* > vecNodes;

        //ds set vector
        _setNodesRecursive( m_pRoot, vecNodes );

        //ds free nodes
        for( const tBSNode* pNode: vecNodes )
        {
            delete pNode;
        }

        /*ds free memory for descriptors
        for( const CDescriptor* pDescriptor: m_pRoot->vecDescriptors )
        {
            delete pDescriptor;
        }*/

        //ds clear nodes vector
        vecNodes.clear( );
    }

    void _setNodesRecursive( const tBSNode* p_pNode, std::vector< const tBSNode* >& p_vecNodes ) const
    {
        //ds must not be zero
        assert( 0 != p_pNode );

        //ds add the current node
        p_vecNodes.push_back( p_pNode );

        //ds check if there are leafs
        if( p_pNode->bHasLeaves )
        {
            //ds add leafs and so on
            _setNodesRecursive( static_cast< const tBSNode* >( p_pNode->pLeafOnes ), p_vecNodes );
            _setNodesRecursive( static_cast< const tBSNode* >( p_pNode->pLeafZeros ), p_vecNodes );
        }
    }

};

#endif //CBSTREE_HPP
