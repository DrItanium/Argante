# Some handy rules for making images
LAC=../LAC/lac
NAGT=../compiler/nagt

# A2 Images
%.img: %.agt
	$(NAGT) $^ $@

%.img: $(SRCDIR)/%.agt
	$(NAGT) $^ $@

# A2 Objects (unlinked)
%.ao: %.agt
	$(NAGT) -c $^ $@

%.ao: $(SRCDIR)/%.agt
	$(NAGT) -c $^ $@

# LACRETNI
%.agt: %.lac
	$(LAC) $^ $@

%.agt: $(SRCDIR)/%.lac
	$(LAC) $^ $@

