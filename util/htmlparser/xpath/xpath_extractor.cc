// Copyright 2014 Easou Inc. All Rights Reserved.
// Author: shunping_ye@staff.easou.com (Ye Shunping)

#include "util/htmlparser/xpath/xpath_extractor.h"

#include <utility>

#include "base/ns.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "util/htmlparser/htmlparser/html_attr.h"
#include "util/htmlparser/htmlparser/html_extractor.h"
#include "util/htmlparser/htmlparser/html_parser.h"
#include "util/htmlparser/htmlparser/html_tree.h"
#include "util/htmlparser/htmlparser/html_dom.h"

DEFINE_int32(max_xpath_result, 5000, "");
DEFINE_int32(max_page_size, 2 * 1024 * 1024, "");

#define XPATH_LIST_SIZE 10

namespace util {

XpathExtractor::XpathExtractor() {
  xpath_result_ = (XPATH_RESULT_NODE*) malloc(
      sizeof(XPATH_RESULT_NODE) * FLAGS_max_xpath_result);
  CHECK(xpath_result_);

  result_buffer_ = new char[FLAGS_max_page_size];
  CHECK(result_buffer_);
}

XpathExtractor::~XpathExtractor() {
  delete[] result_buffer_;
  free(xpath_result_);

  // Delete xpath_map_;
  hash_map<std::string, XpathCacheNode>::iterator it = xpath_map_.begin();
  for (; it != xpath_map_.end(); ++it) {
    freexpath(it->second.list);
    delete[] it->second.list;
  }
}

bool XpathExtractor::ExtractorXpathNode(html_node_t* htmlnode,
                                        const std::string& xpath,
                                        std::vector<XPATH_RESULT_NODE>* result){
  XpathCacheNode node;
  if (!GetXpathNode(xpath, &node)) {
    return false;
  }
  int result_len = FLAGS_max_xpath_result;
  xpathselect(htmlnode, node.list, node.size, xpath_result_, result_len);
  for (int i = 0; i < result_len; ++i) {
    result->push_back(xpath_result_[i]);
  }
  return true;
}

bool XpathExtractor::ExtractorXpath(html_node_t* htmlnode,
                                       const string& xpath,
                                       vector<string>* result) {
  XpathCacheNode node;
  GetXpathNode(xpath, &node);
  return InnerExtractorXpath(htmlnode, node.list, node.size, result);
}

bool XpathExtractor::GetXpathNode(const string& xpath, XpathCacheNode* node) {
  hash_map<std::string, XpathCacheNode>::iterator it = xpath_map_.find(xpath);
  if (it == xpath_map_.end()) {
    xpathnode** xpathlist = new xpathnode*[XPATH_LIST_SIZE];
    for (int i = 0; i < XPATH_LIST_SIZE; ++i) {
      xpathlist[i] = NULL;
    }
    int xpath_len = XPATH_LIST_SIZE;
    CHECK(xpathlist) << "xpathlist is wrong";

    int res = parserxpath(const_cast<char *>(xpath.c_str()), xpathlist,
                          &xpath_len);
    if (res != 0) {
      freexpath(xpathlist);
      delete[] xpathlist;
      LOG(ERROR)<< "Fail to parse xpath:" << xpath;
      return false;
    }
    XpathCacheNode node;
    node.list = xpathlist;
    node.size = xpath_len;
    std::pair<hash_map<std::string, XpathCacheNode>::iterator, bool> ret =
        xpath_map_.insert(make_pair(xpath, node));
    if (!ret.second) {
      freexpath(xpathlist);
      delete[] xpathlist;
      LOG(ERROR)<< "Bad xpath:" << xpath;
      return false;
    } else {
      it = ret.first;
    }
  }
  *node = it->second;
  return true;
}

bool XpathExtractor::ExtractorXpath(const string& html,
                                    const string& xpath,
                                    vector<string>* result) {
  XpathCacheNode node;
  if (!GetXpathNode(xpath, &node)) {
    return false;
  }

  char* page = const_cast<char*>(html.c_str());
  int page_len = html.size();

  html_tree_t *html_tree = html_tree_create(FLAGS_max_page_size);
  if (0 == html_tree_parse(html_tree, page, page_len, true)) {
    LOG(ERROR)<< "html_tree_parse fail\n";
    return false;
  }
  bool ret = InnerExtractorXpath(&html_tree->root, node.list, node.size, result);
  html_tree_del(html_tree);
  return ret;
}

void XpathExtractor::ClearResultBuffer() {
  memset(result_buffer_, 0, sizeof(FLAGS_max_page_size));
}

html_tree_t* ParseDomTree(const string& html) {
  char* page = const_cast<char*>(html.c_str());
  int page_len = html.size();

  html_tree_t *html_tree = html_tree_create(512 * 1024);
  if (0 == html_tree_parse(html_tree, page, page_len, true)) {
    LOG(ERROR)<< "html_tree_parse fail";
    html_tree_del(html_tree);
    return NULL;
  }
  return html_tree;
}

void FreeDomTree(html_tree_t* dom_tree) {
  html_tree_del(dom_tree);
}

bool XpathExtractor::InnerExtractorXpath(html_node_t* root,
                                         xpathnode ** xpathlist, int xpathlength,
                                         std::vector<string>* result) {
  int result_len = FLAGS_max_xpath_result;
  xpathselect(root, xpathlist, xpathlength, xpath_result_, result_len);


  for (int i = 0; i < result_len; i++) {
    if (xpath_result_[i].type == 0) {
      ClearResultBuffer();
      int len = html_node_extract_content(static_cast<html_node_t*>(xpath_result_[i].node),
                                          result_buffer_, FLAGS_max_page_size);
      if (len > 0) {
        result->push_back(result_buffer_);
      }
    } else if (xpath_result_[i].type == 1) {
      char* attr = ((html_attribute_t *) (xpath_result_[i].node))->value;
      if (attr != NULL) {
        result->push_back(attr);
      }
    }
  }
  return true;
}

bool XpathExtractor::ExtractorXpathNodeAttr(const std::string& html,
                                            const std::string& xpath,
                                            const std::string& attr,
                                            std::vector<std::string>* result) {
  html_tree_t* html_tree = util::ParseDomTree(html);
  if (html_tree == NULL) {
    LOG(INFO) << "html tree null";
    return false;
  }

  vector<XPATH_RESULT_NODE> nodes;
  ExtractorXpathNode(&html_tree->root, xpath, &nodes);
  if (!nodes.empty()) {
    for (size_t i = 0; i != nodes.size(); ++i) {
      html_node_t* node = (html_node_t*)nodes[i].node;
      // string tag_name = node->html_tag.tag_name;
      while (node->html_tag.attribute != NULL) {
        string attr_name = node->html_tag.attribute->name;
        if (attr_name == attr) {
          string value = node->html_tag.attribute->value;
          result->push_back(value);
          break;
        }   
        node->html_tag.attribute = node->html_tag.attribute->next;
      }
    }
  } else {
    return false;
  }
  FreeDomTree(html_tree);
  return true;
}
}
