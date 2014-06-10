Version:parser1.3.2.7.1

版本特性：
css server interface chnage. 
1. add free interface.
2. add global lock to make sure it will be init only once in one process.

变化：
include/easou_vhtml_tree.h
lib/libvhtmlparser.a

=======================================================================

Version:parser1.3.2.7

版本特性：
1.将版本parser1.3.2.6与版本parser1.3.1.9 merge。
  主要是在parser1.3.2.6的基础上增加了抽取完整链接地址、增加链接类型-IFRAME以及图片链接的提取;
  另外，对xpath接口做了小修改-将easou_xpath.cpp设置的1000改为5000

=======================================================================
Version: parser1.3.2.6

版本特性：
1. 增加css文件获取统计信息
M       easou_vhtml_tree.cpp
M       easou_vhtml_basic.h

=======================================================================
Version: parser1.3.2.5

版本特性：
1. tmplt_apply_exreg_rule暴露为接口，和一些问题修改
2. tmplt_extract_result_t增加ex_reg成员
以上接口改变主要是方便在使用node list时，也能应用模板的正则排除规则。

修改了	extractor/easou_extractor_template.h
	extractor/easou_extractor_template.cpp

=======================================================================
Version: parser1.3.2.4

版本特性：
1. 标题抽取case修改，去掉realtitle抽取尾部的( { 字符
修改了 extractor/easou_extractor_com.cpp
2. 模版抽取case修改，修改个别页面内容抽取不到的问题
3. coredump修改，修改极少数情况下会出现的assert fail问题（同parser1.3.1.7中的问题修改）
修改了 extractor/easou_extractor_template.cpp
4. 注释掉一些日志输出
修改了 cssparser/easou_css_parser_inner.cpp

=======================================================================
Version: parser1.3.2.3

版本特性：
1. 修改parser1.3.2.2中模版抽取偶尔出core的问题
修改了	extractor/easou_extractor_template.cpp

2. 针对吧首页改进realtitle提取，比如：斗破苍穹吧， realtitle提取为 斗破苍穹
修改了	extractor/easou_extractor_title.h
	extractor/easou_extractor_title.cpp

=======================================================================
Version: parser1.3.2.2

版本特性：
1. 完善模版抽取的下一页接口，和其它一些问题改进。

效率测试：
1. 模板初始化时间 <1s（理想情况下测试值500ms左右），如果mysql负载很高，可能时间更久。
2. 是否命中模板 < 1ms
3. 模板抽取 < 1ms

allnum:2100 alltime:11627972us avg:5537.13us
tmplt_try:2100 tmplt_no_hit:1999 tmplt_rt_fail:11 tmplt_ac_fail:4 tmplt_hit:86 tmlt_pic_num:253
all:2100
               area_partition: avg:411.32    us
              html_tree_parse: avg:799.64    us
   html_vtree_parse_with_tree: avg:792.21    us
               mark_area_tree: avg:2280.69   us
                        parse: avg:4300.30   us
                template:entry avg:44.35     us
              template:extract avg:14.46     us

====================同parser1.3.2.1比较，修改的有======================
htmlparser
	easou_html_extractor.h|cpp 增加int html_tree_extract_link(html_node_list_t* list, char* baseUrl, link_t* link, int& num);方法
extractor
	修改了easou_extractor_template.h|cpp
	
以下未修改
utils
cssparser
vhtmlparser
ahtmlparser
markparser
interface
pagetype
bbsparser
linktype

===================备注=======================
适用于pageclassify1.5.0.1及以上版本
适用于wapResClassify2.1.0.1及以上版本
