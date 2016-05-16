#ifndef CBSTMatch_HPP
#define CBSTMatch_HPP



//ds elementary match object: inspired by opencv cv::DMatch (docs.opencv.org/trunk/d4/de0/classcv_1_1DMatch.html)
struct CBSTMatch
{
    CBSTMatch( const uint64_t& p_uIDQUERYDescriptor,
               const uint64_t& p_uIDTRAINDescriptor,
               const uint32_t& p_uIDTRAINTree,
               const uint32_t& p_uMatchingDistanceHAMMING ): uIDQUERYDescriptor( p_uIDQUERYDescriptor ),
                                                             uIDTRAINDescriptor( p_uIDTRAINDescriptor ),
                                                             uIDTRAINTree( p_uIDTRAINTree ),
                                                             uMatchingDistanceHAMMING( p_uMatchingDistanceHAMMING )
    {
        //ds nothing to do
    }

    CBSTMatch( const uint64_t& p_uIDQUERYDescriptor,
               const uint64_t& p_uIDTRAINDescriptor,
               const uint32_t& p_uMatchingDistanceHAMMING ): uIDQUERYDescriptor( p_uIDQUERYDescriptor ),
                                                             uIDTRAINDescriptor( p_uIDTRAINDescriptor ),
                                                             uIDTRAINTree( 0 ),
                                                             uMatchingDistanceHAMMING( p_uMatchingDistanceHAMMING )
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
    const uint32_t uMatchingDistanceHAMMING;

};

#endif //CBSTMatch_HPP
