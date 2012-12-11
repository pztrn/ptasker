SUBDIRS = po \
	src \
	tests

dist_doc_DATA = AUTHORS \
	COPYING \
	INSTALL \
	NEWS \
	NEWS.html \
	README \
	README.html

README.html: README
	asciidoc -n -a toc README

NEWS.html: NEWS
	asciidoc -n -a toc NEWS
