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


/**
 * @return 1 if point (x, y) locates at the rect's area or on the left line
 *           or the top line.
 * @return 0 if point (x, y) locates out of the rect's area or on the right
 *           line or the bottom line.
 */
int sf_rect_iscontain(struct sf_rect *r, int x, int y);

int sf_rect_isintersect(struct sf_rect *a, struct sf_rect *b);


#endif /* SF_RECT_H */
