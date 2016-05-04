NAME=KacstOne
VERSION=5.0

DIST=kacst_one_$(VERSION)

FF=fontforge -lang=ff
FFLAGES=0x200000&0x80 # round, opentype
SCRIPT='Open($$1); SetFontNames("","","","","","$(VERSION)"); Generate($$2,"",$(FFLAGES))'

FONTS=KacstOne KacstOne-Bold
DOC=NEWS Copyright LICENSE BUGS README ChangeLog

SFD=$(FONTS:%=%.sfd)
TTF=$(FONTS:%=%.ttf)

all: ttf

ttf: $(TTF)

%.ttf: %.sfd
	@echo "Building $@"
	@$(FF) -c $(SCRIPT) $< $@ 2>/dev/stdout 1>/dev/stderr | tail -n +4

dist: $(TTF)
	@echo "Making dist tarball"
	@mkdir -p $(DIST)
	@cp $(SFD) $(TTF) $(DOC) Makefile $(DIST)
	@tar cvfj $(DIST).tar.bz2 $(DIST)

clean:
	@rm -rf $(TTF) $(DIST) $(DIST).tar.bz2
