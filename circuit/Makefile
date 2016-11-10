PAPER=A4
DPI=600

NAME=avrlirc

# Create multiple copies of one PCB
MPCB_TARGETS=$(NAME) $(NAME) $(NAME)
MPCB_OPTION  =
MPCB_OPTION += -geometry +0+0  # Space betwee PCB
MPCB_OPTION += -border 2 -bordercolor black
#MPCB_OPTION+=-tile 2x1


SCH_TARGETS = $(NAME)
PCB_FILES = $(basename $(wildcard *.pcb))

SCH_PDF_TARGETS = $(addsuffix -sch.pdf, $(SCH_TARGETS)) 

PCB_TARGETS  = $(addsuffix -bottom.png, $(PCB_FILES))
PCB_TARGETS += $(addsuffix -bottom-mirrored.png, $(PCB_FILES))
PCB_TARGETS += $(addsuffix -top-assembly.png, $(PCB_FILES))
PCB_TARGETS += mpcb-bottom.png

.PHONY: pcb clean
all: $(PCB_TARGETS) schpdf avrlirc-01-sch.png

$(SCH_PDF_TARGETS): %-sch.pdf: $(wildcard $(%)*.sch)
	gaf export -f PDF -o $@ $(sort $^)

%-sch.png: %.sch
	gaf export -f PNG -d $(DPI) --no-color -o $@ $<	

schpdf: $(addsuffix -sch.pdf, $(SCH_TARGETS))

%-bottom.png: %.pcb
	pcb -x png --dpi $(DPI)  --drill-helper \
	  --layer-stack bottom,pins,vias \
	  --monochrome --as-shown \
	  --outfile $@ $<
	# Fix wrong DPI information
	mogrify -units PixelsPerInch -density $(DPI) $@	

%-mirrored.png: %.png
	convert $< +flip $@

%-top-assembly.png: %.pcb %-bottom.png
	pcb -x png --dpi $(DPI) --use-alpha \
	  --layer-stack outline,elements,silk,pins,vias,top \
	  --monochrome  --as-shown \
	  --outfile $@ $<
	# Fix wrong DPI information
	mogrify -units PixelsPerInch -density $(DPI) $@	
	composite -dissolve 170 $@ $*-bottom.png -alpha Set $@

mpcb-bottom.png: $(addsuffix -bottom.png, $(MPCB_TARGETS))
	montage $(addsuffix -bottom.png, $(MPCB_TARGETS))  $(MPCB_OPTION) $@
	mogrify -units PixelsPerInch -density $(DPI) $@	

%-bottom.pdf: %.pcb
	pcb -x ps --multi-file --mirror --media $(PAPER) --psfile $*.ps $<
	ps2pdf $*.bottom.ps $@



clean:
	- rm -f $(SCH_PDF_TARGETS)
	- rm -f $(addsuffix *-sch*.png, $(SCH_TARGETS))
	rm -f *.sch~
	rm -f $(PCB_PNG_TARGETS)
	rm -f *-bottom.pdf *.*.ps
	- rm -f *.pcb-
	

