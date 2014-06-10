/**
 * @file easou_lang_relate.h
 * @brief 语言相关部分.
 *  
 **/


#ifndef  __EASOU_LANG_RELATE_H_
#define  __EASOU_LANG_RELATE_H_
#include <string.h>

extern const char * for_interaction[] ; 

extern const char * for_friend_link_inanchor[] ; 


extern const char * for_friend_link_title[] ;


extern const char *for_friend_link_no[] ;

extern const char *for_rel_link[] ;

extern const char * hot_rank[] ;

extern const char * mypos_filter_words[]; 

extern const char * mypos_hint[]; 

extern const char *mypos_mark_filter_words[]; 

extern const char * topanchor[];
extern const char * typeword[];

extern const char * no_nav_anchor[];

extern const char *ARTI_SIGN_FEAT_TERM[];

extern const char *COPYRIGHT_ANCHOR_TERM[];

extern const char *COPYRIGHT_BAD_TEAM[];

extern const char *COPYRIGHT_FEAT_TERM[];//文字,链接,链接组,文字链接

inline bool is_nonsense_word_use_in_realtitle(const char *word)
{
	if(strcmp(word,"的") == 0
			|| word[0] == ','){
		return true;
	}

	return false;
}


#endif  //__EASOU_LANG_RELATE_H_

