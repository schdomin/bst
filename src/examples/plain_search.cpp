#include <iostream>
#include "CBSTree.hpp"

//ds current setup (left for clarity, could also be omitted for standard settings)
#define MAXIMUM_DISTANCE_HAMMING 25
#define BSTREE_MAXIMUM_DEPTH 75
#define DESCRIPTOR_SIZE_BITS 256



int32_t main( int32_t argc, char** argv )
{
    //ds train descriptor pool
    const std::shared_ptr< std::vector< CDescriptorBRIEF< DESCRIPTOR_SIZE_BITS > > > vecDescriptorPoolTRAIN = CBSNode< BSTREE_MAXIMUM_DEPTH, DESCRIPTOR_SIZE_BITS >::getRandomDescriptors< 10000 >( );
    
    //ds allocate a BTree object on these descriptors
    const CBSTree< MAXIMUM_DISTANCE_HAMMING, BSTREE_MAXIMUM_DEPTH, DESCRIPTOR_SIZE_BITS > cBTree( 0, vecDescriptorPoolTRAIN );
    


    //ds query descriptor pool
    const std::shared_ptr< std::vector< CDescriptorBRIEF< DESCRIPTOR_SIZE_BITS > > > vecDescriptorPoolQUERY = CBSNode< BSTREE_MAXIMUM_DEPTH, DESCRIPTOR_SIZE_BITS >::getRandomDescriptors< 10000 >( );

    //ds get matches (opencv IN/OUT style)
    std::vector< CBSTMatch > vecMatches1;
    cBTree.match( vecDescriptorPoolQUERY, vecMatches1 );

    //ds get matches directly
    const std::shared_ptr< std::vector< CBSTMatch > > vecMatches2 = cBTree.getMatches( vecDescriptorPoolQUERY );



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

    //ds done
    std::cout << "successfully matched descriptors: " << vecMatches1.size( ) << std::endl;
    return 0;
}
