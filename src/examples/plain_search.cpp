#include <iostream>

#include "../types_core/CBSTree.hpp"

//ds current setup (left for clarity, could also be omitted for standard settings)
#define MAXIMUM_DISTANCE_HAMMING 25
#define BSTREE_MAXIMUM_DEPTH 75
#define DESCRIPTOR_SIZE_BITS 256
typedef CDescriptorBinary< DESCRIPTOR_SIZE_BITS > CDescriptor;
typedef CBSNode< CDescriptor, BSTREE_MAXIMUM_DEPTH > CNode;
typedef CBSTree< CNode, MAXIMUM_DISTANCE_HAMMING > CTree;

//ds dummy descriptor generation
const std::shared_ptr< const std::vector< const CDescriptor* > > getDummyDescriptors( const uint64_t& p_uNumberOfDescriptors )
{
    //ds preallocate vector
    std::vector< const CDescriptor* > vecDescriptors( p_uNumberOfDescriptors );

    //ds set values
    for( uint64_t uID = 0; uID < p_uNumberOfDescriptors; ++uID )
    {
        vecDescriptors[uID] = new CDescriptor( uID, CDescriptor::CDescriptorValues( ) );
    }

    //ds done
    return std::make_shared< const std::vector< const CDescriptor* > >( vecDescriptors );
}



int32_t main( int32_t argc, char** argv )
{
    //ds train descriptor pool
    const std::shared_ptr< const std::vector< const CDescriptor* > > vecDescriptorPoolTRAIN = getDummyDescriptors( 10000 );
    
    //ds allocate a BTree object on these descriptors (no shared pointer passed as the tree will have its own constant copy of the train descriptors)
    const CTree cBTree( 0, *vecDescriptorPoolTRAIN );
    
    //ds query descriptor pool
    const std::shared_ptr< const std::vector< const CDescriptor* > > vecDescriptorPoolQUERY = getDummyDescriptors( 5000 );



    //ds get matches (opencv IN/OUT style)
    std::vector< CBSTMatch > vecMatches1;
    cBTree.match( vecDescriptorPoolQUERY, vecMatches1 );

    //ds get matches directly
    const std::shared_ptr< const std::vector< CBSTMatch > > vecMatches2 = cBTree.getMatches( vecDescriptorPoolQUERY );



    //ds matches must be identical: check if number of elements differ
    if( vecMatches1.size( ) != vecMatches2->size( ) )
    {
        std::cerr << "received inconsistent matching returns" << std::endl;
        return -1;
    }

    //ds check each element
    for( uint64_t uIndexMatch = 0; uIndexMatch < vecMatches1.size( ); ++uIndexMatch )
    {
        //ds check if not matching
        if( vecMatches1[uIndexMatch].uIDQUERYDescriptor       != vecMatches2->at( uIndexMatch ).uIDQUERYDescriptor       ||
            vecMatches1[uIndexMatch].uIDTRAINDescriptor       != vecMatches2->at( uIndexMatch ).uIDTRAINDescriptor       ||
            vecMatches1[uIndexMatch].uIDTRAINTree             != vecMatches2->at( uIndexMatch ).uIDTRAINTree             ||
            vecMatches1[uIndexMatch].uMatchingDistanceHAMMING != vecMatches2->at( uIndexMatch ).uMatchingDistanceHAMMING )
        {
            std::cerr << "received inconsistent matching returns" << std::endl;
            return -1;
        }
    }

    //ds never support memory leaks
    for( const CDescriptor* pDescriptor: *vecDescriptorPoolTRAIN )
    {
        delete pDescriptor;
    }
    for( const CDescriptor* pDescriptor: *vecDescriptorPoolQUERY )
    {
        delete pDescriptor;
    }

    //ds done
    std::cout << "successfully matched descriptors: " << vecMatches1.size( ) << std::endl;
    return 0;
}
