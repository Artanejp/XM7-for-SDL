# Rules of compiling, maybe common.
# (C)2013 K.Ohta
#

#Deps
ifndef DPATH
DPATH = $(realpath ..)
endif

$(OBJS_RELEASE): $(DPATH)/config.mak \
	$(DPATH)/common.mak \
	$(DPATH)/rules.mak 

$(OBJS_DEBUG): $(DPATH)/config.mak \
	$(DPATH)/common.mak \
	$(DPATH)/rules.mak 


# Rules
Release/%.o: %.c 
	@mkdir -p Release
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/%.o: %.cpp
	@mkdir -p Release
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Debug/%.o: %.c
	@mkdir -p Debug
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)



Debug/%.o: %.cpp
	@mkdir -p Debug
	$(CXX) ${CXXFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)



.asm.o:
	nasm ${ASFLAGS} $<


Debug/%.d: %.c
	@mkdir -p Debug/
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS_DEBUG) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,Debug\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

Debug/%.d: %.cpp
	@mkdir -p Debug/
	set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS_DEBUG) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,Debug\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

Release/%.d: %.c
	@mkdir -p Release/
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS_RELEASE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,Release\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

Release/%.d: %.cpp
	@mkdir -p Release/
	set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS_RELEASE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,Release\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


