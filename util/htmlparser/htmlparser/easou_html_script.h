
#ifndef  __EASOU_HTML_SCRIPT_H_
#define  __EASOU_HTML_SCRIPT_H_

/*
 * @brief Limited ECMAScript evaluation
 * @param[in] script The ECMAScript
 * @param[out] buf Output buffer
 * @param[in] bufsize Size of output buffer
 * @return If success return 0, otherwize return -1
 */
int js_eval(const char *script, char *buf, int bufsize);

#endif  /* __EASOU_HTML_SCRIPT_H_ */

