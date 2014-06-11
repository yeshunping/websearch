

#ifndef EASOU_COMMON_UTILS_H_
#define EASOU_COMMON_UTILS_H_

/**
 * @brief check path
 * @return:
 * 2, is a file
 * 1, path not exist
 * 0, is a directory
 * -1, not a directory or unrecognized
 */
int check_path(const char * dirpath);

#endif /* EASOU_COMMON_UTILS_H_ */
