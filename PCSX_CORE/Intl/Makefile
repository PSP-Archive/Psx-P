#
# PCSX Makefile for Linux
#

all: pofile update-po build

.SUFFIXES: .po .mo
LANGUAGES = fr_FR.po it_IT.po

SRCS = ../*.c ../ix86/*.c ../Linux/*.c ../Win32/*.c

pofile:
	xgettext --keyword=N_ --keyword=_ --output=pcsx.po ${SRCS}

update-po:
	for lang in ${LANGUAGES} ; do \
	    msgmerge $$lang pcsx.po > $$lang-new ; \
	    mv $$lang-new $$lang ; \
	done

build: ${LANGUAGES:%.po=%.mo}
	for lang in ${LANGUAGES:%.po=%} ; do \
	    mkdir -p Langs/$$lang/LC_MESSAGES ; \
	    mv $$lang.mo Langs/$$lang/LC_MESSAGES/pcsx.mo ; \
	done

.po.mo:
	msgfmt -c -o $@ $<

.PHONY: pofile update-po build

