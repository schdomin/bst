#ifndef CBSTMatch_HPP
#define CBSTMatch_HPP



//ds elementary match object: inspired by opencv cv::DMatch (docs.opencv.org/trunk/d4/de0/classcv_1_1DMatch.html)
template< typename tPrecision = double >
struct CBSTMatch
{
    CBSTMatch( const uint64_t& p_uIDQUERYDescriptor,
               const uint64_t& p_uIDTRAINDescriptor,
               const uint32_t& p_uIDTRAINTree,
               const tPrecision& p_dMatchingDistance ): uIDQUERYDescriptor( p_uIDQUERYDescriptor ),
                                                        uIDTRAINDescriptor( p_uIDTRAINDescriptor ),
                                                        uIDTRAINTree( p_uIDTRAINTree ),
                                                        dMatchingDistance( p_dMatchingDistance )
    {
        //ds nothing to do
    }

    CBSTMatch( const uint64_t& p_uIDQUERYDescriptor,
               const uint64_t& p_uIDTRAINDescriptor,
               const tPrecision& p_dMatchingDistance ): uIDQUERYDescriptor( p_uIDQUERYDescriptor ),
                                                        uIDTRAINDescriptor( p_uIDTRAINDescriptor ),
                                                        uIDTRAINTree( 0 ),
                                                        dMatchingDistance( p_dMatchingDistance )
    {
        //ds nothing to do
    }

    ~CBSTMatch( )
    {
        //ds nothing to do
    }

    const uint64_t uIDQUERYDescriptor;
    const uint64_t uIDTRAINDescriptor;
    const uint32_t uIDTRAINTree;
    const tPrecision dMatchingDistance;

};

#endif //CBSTMatch_HPP
