
#include <string.h>
#include "easou_html_dom.h"
#include "easou_html_script.h"
#include "easou_string.h"

#define MAX_SCRIPT_TOKEN_LEN 256

/**
 * @brief is whitespace
 **/
#define IS_WS(ch) (g_whitespace_map[(unsigned char)(ch)])

static const char *javascript_key[] =
{ "function", "if", "else", "switch", "for", "while" };

int js_eval(const char *script, char *htmlbuf, int bufsize)
{
	const char *p = script;
	int len = strlen("document.write");
	char scanbuf[MAX_SCRIPT_TOKEN_LEN];
	int pos = 0;
	int bufpos = 0;
	int key_size = 0;
	char mark = 0;
	char line_break = 0;
	char bend = 0;
	int state = 0;
	int i = 0;

	// -1 : error
	// 0 : normal scan;
	// 1 : begin scan,have meet '+' or '('
	// 2 : meet a mark "\"" or "\'"

	if (!p || !htmlbuf || bufsize <= 0)
		return 0;

	*htmlbuf = 0;

	key_size = sizeof(javascript_key) / sizeof(char *);

	while (*p)
	{
		switch (state)
		{
		case 0:
		{
			//<!--..--> is html comment but not script comment
			if (strncmp(p, "<!--", strlen("<!--")) == 0)
			{
				p += strlen("<!--");
				break;
			}
			if (strncmp(p, "-->", strlen("-->")) == 0)
			{
				p += strlen("-->");
				break;
			}

			if (!IS_WS(*p) && *p != '(' && *p != '{')
			{
				if (pos + 1 < (int) sizeof(scanbuf))
					scanbuf[pos++] = *p;
				p++;
				break;
			}

			if (pos < 2)
			{ // too short token
				pos = 0;
				p++;
				break;
			}

			//get a chunk
			scanbuf[pos] = 0;

			for (i = 0; i < key_size; i++)
			{
				if (strncasecmp(javascript_key[i], scanbuf, strlen(javascript_key[i])) == 0)
				{ //can not process
					bend = 1;
					state = 0;
					bufpos = 0;

					break;
				}
			}

			if (bend)
				break;

			line_break = 0;

			while (*p && IS_WS(*p))
				p++;

			if ((pos != len && pos != len + 2)
					|| (strncasecmp("document.writeln", scanbuf, len + 2) && strncasecmp("document.write", scanbuf, len)))
			{

				bend = 1;
				bufpos = 0;

				break;
			}

			if (*p != '(')
			{
				p++;
				break;
			}

			if (pos == len + 2)
				line_break = 1;

			p++;
			pos = 0;
			scanbuf[pos] = 0;
			state = 1;

			break;
		}
		case 1: //have meet '+' or '('
		{
			while (*p && IS_WS(*p))
				p++;

			if (*p != '\"' && *p != '\'')
			{ //can not process
				bend = 1;
				bufpos = 0;
				state = 0;

				break;
			}

			mark = *p++;
			state = 2;

			break;
		}
		case 2:
		{
			//record the html src
			while (*p && (*p != mark||*p == mark&&*(p-1)!='\\'))
			{
				if (bufpos + 1 >= bufsize)
				{
					p++;
					continue;
				}
				htmlbuf[bufpos++] = *p++;
				if (*p == '<' && strncasecmp("script", p + 1, 6) == 0)
				{ //omit the embed case
					break;
				}
			}

			if (*p != mark)
			{ // mark mismatch,error
				mark = 0;
				bend = 1;
				bufpos = 0;
				state = 0;

				break;
			}

			mark = 0;
			htmlbuf[bufpos] = 0;

			//mark  match
			p++;

			while (*p && IS_WS(*p))
				p++;
			if (*p == 0)
			{ //"(" mismatch ,error
				bend = 1;
				bufpos = 0;
				state = 0;

				break;
			}

			//"(" match
			if (*p == ')')
			{
				if (line_break && bufpos + 1 < bufsize)
				{ // insert '\n'
					htmlbuf[bufpos++] = '\n';
					htmlbuf[bufpos] = 0;
				}

				line_break = 0;
				state = 0;
				p++;

				while (*p && (IS_WS(*p) || *p == ';'))
					p++;

				break;
			}

			if (*p == '+')
			{
				state = 1;
				p++;
			}
			else
			{
				bend = 1;
				bufpos = 0;
				state = 0;
			}

			break;
		}
		default:
			break;
		}
		if (bend) //end
			break;
	}

	for (i = 0; i < bufpos; i++)
	{
		if (htmlbuf[i] == '<' && strncasecmp("script", htmlbuf + i + 1, 6) == 0)
		{
			bufpos = 0;
			break;
		}
	}

	htmlbuf[bufpos] = 0;

	return bufpos;
}

