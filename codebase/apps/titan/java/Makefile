# Makefile for TrControl

JAVA_CLASSES = $(JAVA_SRCS:.java=.class)
JAVAC = javac
JAVA_FLAGS = -classpath $(CLASSPATH)

JAVA_SRCS = \
	AScope.java \
	AScopeInfoFrame.java \
	AScopeParams.java \
	AntennaControl.java \
	AntennaPosnDisplay.java \
	AsciiMessage.java \
	BeamGeomControl.java \
	BeamMessage.java \
	CalibrationParams.java \
	Communicator.java \
	ControlPanel.java \
	ControlPanelParams.java \
	CustomBorder.java \
	DisplayParams.java \
	MainMenu.java \
	MainWindowParams.java \
	MessageQueue.java \
	NFormat.java \
	OnOffPanel.java \
	Parameters.java \
	PowerControl.java \
	PpiDisplay.java \
	PpiDisplayParams.java \
	RadarParams.java \
	ScanParams.java \
	Simulator.java \
	StatusMessage.java \
	SunPosnDisplay.java \
	SunTrack.java \
	TimeManager.java \
	TrControl.java \
	WorldPlot.java

#	Gutil.java \

all: $(JAVA_CLASSES)

clean:
	/bin/rm -f $(JAVA_CLASSES) *.class
	cd params; make clean

jar:
	jar cmf Manifest ../TrControl.jar *

.SUFFIXES: .java .class

.java.class:
	$(JAVAC) $(JAVA_FLAGS) $<

