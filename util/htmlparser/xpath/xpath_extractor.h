// Copyright 2013 Easou Inc. All Rights Reserved.
// Author: lucus_xu@staff.easou.com (Liqiu Xu)

#ifndef UTIL_XPATH_XPATH_EXTRACTOR_H_
#define UTIL_XPATH_XPATH_EXTRACTOR_H_

#include <string>
#include <vector>

#include "base/hash_tables.h"
#include "util/htmlparser/htmlparser/xpath.h"

namespace util {

// Helper function to get domtree using html
// If return value is not NULL, it should be deleted by caller using
// html_tree_del(html_tree);
html_tree_t* ParseDomTree(const std::string& html);
void FreeDomTree(html_tree_t* dom_tree);

class XpathExtractor {
 public:
  XpathExtractor();
  ~XpathExtractor();

  // If you pass html instead of html_node_t, this function will parse it to dom tree,
  // then extract xpath result from that tree.
  // For performance, you should pass html_node_t instead of html.
  bool ExtractorXpath(const std::string & html,
                      const std::string& xpath,
                      std::vector<std::string>* result);

  bool ExtractorXpath(html_node_t* htmlnode,
                      const std::string& xpath,
                      std::vector<std::string>* result);

  // Return matched nodes, you can extract information from its content, attributes, or event
  // get info from its children nodes.
  bool ExtractorXpathNode(html_node_t* htmlnode,
                          const std::string& xpath,
                          std::vector<XPATH_RESULT_NODE>* result);

  bool ExtractorXpathNodeAttr(const std::string & html,
                              const std::string& xpath,
                              const std::string& attr,
                              std::vector<std::string>* result);
 private:
  struct XpathCacheNode {
    xpathnode **list;
    int size;
  };
  void ClearResultBuffer();
  bool InnerExtractorXpath(html_node_t* htmlnode,
                           xpathnode ** xpathlist, int xpathlength,
                           std::vector<std::string>* result);
  bool GetXpathNode(const std::string& xpath, XpathCacheNode* node);

  char* result_buffer_;
  XPATH_RESULT_NODE* xpath_result_;
  base::hash_map<std::string, XpathCacheNode> xpath_map_;
};
}
#endif  // UTIL_XPATH_XPATH_EXTRACTOR_H_
