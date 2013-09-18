#ifndef SF_RECT_H
#define SF_RECT_H


/**
 *  (x,y)
 *    *------*
 *    | rect | h
 *    *------*
 *       w
 */
struct sf_rect {
    int x;
    int y;
    int w;
    int h;
};


int sf_rect_iscontain(struct sf_rect *r, int x, int y);


#endif /* SF_RECT_H */
