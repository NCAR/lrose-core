# Makefile for Params classes

JAVA_CLASSES = $(JAVA_SRCS:.java=.class)
JAVAC = javac
JAVA_FLAGS = -classpath ..:$(CLASSPATH)

JAVA_SRCS = \
	AbstractParam.java \
	AbstractParamGui.java \
	BooleanParam.java \
	BooleanParamGui.java \
	CollectionFrame.java \
	CollectionParam.java \
	CollectionParamGui.java \
	DoubleParam.java \
	DoubleParamGui.java \
	FloatParam.java \
	FloatParamGui.java \
	IntegerParam.java \
	IntegerParamGui.java \
	OptionParam.java \
	OptionParamGui.java \
	ParamGuiInterface.java \
	ParamGuiParams.java \
	ParamInfoFrame.java \
	ParamInterface.java \
	ParamType.java \
	ParamManager.java \
	StringParam.java \
	StringParamGui.java

all: $(JAVA_CLASSES)

clean:
	/bin/rm -f $(JAVA_CLASSES) *.class

clean_all: clean
	/bin/rm -f *.class *~

.SUFFIXES: .java .class

.java.class:
	$(JAVAC) $(JAVA_FLAGS) $<


