#!/usr/bin/env python2
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/10/31 11:52:14 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###############################################################
#
# This script simplifies the task of adding/updating copyright notices
# for source code, scripts, Makefiles, etc. The user can select the
# root of a directory tree to work on, and the script will recursively
# build a list of files to edit (matching configured prefixes and extensions).
# Files can be opened, edited, and saved from within the GUI.
# The script keeps track of which files have been modifed, completed, or marked
# for additional attention ('todo'), using a metadata file stored in the root of
# the directory tree, so an editing session may be resumed later.
#
# Functions:
#   - Selecting a file in the list will automatically open the file for editing in the
#      text box. Text can be added or deleted here.
#   - The insert function runs copyright_prepend on the file, which inserts a default copyright or 
#     text from a file (e.g. a copyright notice) at the top of the file being edited
#   - The remove function runs copyright_remove on the file
#   - 'Save' saves the modified file to disk and marks it appropriately
#   - 'Complete' saves any modifications and marks the file appropriately
#   - 'Todo' marks the file, but does not save changes to disk
#
#  Note that changes will be 'silently' lost if a new file is selected without saving the old first!
#
#  Requires tkinter python module
#
# A.Gaydos 8/2012
#
###############################################################

from Tkinter import *
import tkMessageBox
import tkFileDialog
import commands
import os
import re
import subprocess
from optparse import OptionParser, OptionGroup

# GUI config
WIN_WIDTH = 1000
WIN_HEIGHT = 720
WIN_X = 100
WIN_Y = 20

# make sure copyright_prepend and copyright_remove are on the user's PATH
path_error = False
cmd = ['which', 'copyright_prepend']
p = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
output, errors = p.communicate()

if output == "":
    path_error = True
    print "ERROR: Could not find copyright_prepend script. Please add to your $PATH"

cmd = ['which', 'copyright_remove']
p = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
output, errors = p.communicate()

if output == "":
    path_error = True
    print "ERROR: Could not find copyright_remove script. Please add to your $PATH"

if path_error == True:
    sys,exit(1)

# file extensions that will be excluded for editing
EXCLUDED_FILE_EXTENSIONS = ['jpg','png','gif','zip','tar','gz','doc','docx','ppt','o','a','jar','grb','grb2','nc','ps','so','pdf']

# file types to include (resulting from UNIX 'file' command query
INCLUDED_FILE_TYPES = ['ASCII','script','XML','text', 'FORTRAN']

# forced comment chars - from copyright_prepend options
# a list of [ [ display_name, copyright_prepend_argument ] ]
FORCED_COMMENT_CHARS = [ [ "C style /*", "c" ], [ "F90 !", "f"], [ "java //", "j"],
                         [ "HTML <!--", "l" ], [ "Matlab %", "m"], [ "Fortran C", "o"],
                         [ "C++ //", "p" ], [ "Script #", "s"], ["SQL --", "q"] ]

# string which signifies copyright_prepend should choose the comment style automatically
DEFAULT_COMMENT_CHAR = "<Auto>"

# default file locations (for file and directory choosers)
DEFAULT_DIR = os.environ['HOME'] + "/cvs"

# name of the metadata file that the editor uses to keep track of marks and changes
# this will be created at the top of EACH directory tree selected
CONFIG_FILE_NAME=".copyright_editor.log"

# file list highlighting colors
MODIFIED_COLOR = "darkred"
TODO_COLOR = "blue"
COMPLETED_COLOR = "darkgreen"

WINDOW = Tk()

options = None

class Program:
    """
    Set up the gui and initialize lists and directories
    """
    def __init__ (self):

        # set defaults
        self.files = []
        self.default_dir = DEFAULT_DIR 
        self.directory = ""
        self.copyright_file = ""
        self.fileModified = False

        self.getOptions()

        # set up window
        WINDOW.title("RAL Copyright Editor")
        WINDOW.geometry("%dx%d%+d%+d" % (WIN_WIDTH,WIN_HEIGHT,WIN_X,WIN_Y))

        ############## file chooser frame ###################
        self.files_frame = Frame(WINDOW, bd = 1)
        self.files_frame.pack(side=LEFT, padx=30)

        self.files_label_frame = Frame(self.files_frame, bd = 1)
        self.files_label_frame.pack(side=TOP, padx=30)

        Label(self.files_label_frame,text="Files in this directory tree").pack(side=TOP)
        Label(self.files_label_frame,text="Unmodified  ").pack(side=LEFT)
        Label(self.files_label_frame,text="Modified  ",fg=MODIFIED_COLOR).pack(side=LEFT)
        Label(self.files_label_frame,text="TODO  ",fg=TODO_COLOR).pack(side=LEFT)
        Label(self.files_label_frame,text="Completed  ",fg=COMPLETED_COLOR).pack(side=LEFT)

        # file list
        self.filechooser_frame = Frame(self.files_frame)
        self.filechooser_frame.pack(side=TOP)
        scrollbar = Scrollbar(self.filechooser_frame, orient=VERTICAL)
        self.filelist = Listbox(self.filechooser_frame, yscrollcommand=scrollbar.set,height=35,width=35)
        self.filelist.bind('<<ListboxSelect>>', self.selectFile)
        scrollbar.config(command=self.filelist.yview)
        self.filelist.config(yscrollcommand=scrollbar.set)
        scrollbar.pack(side=RIGHT, fill=Y)
        self.filelist.pack(side=BOTTOM, fill=BOTH, expand=1)
        self.list_scrollbar = scrollbar

        # open directory button        
        self.filechooser_button = Button(self.files_frame, text="Open directory",command=self.chooseDirectory)
        self.filechooser_button.pack(side=LEFT, padx=30)

        ############### editor frame #####################

        # file and path labels
        self.editor_frame = Frame(WINDOW)
        self.editor_frame.pack(fill=X, pady=30)

        Label(self.editor_frame,text="Current file:").pack(side=TOP)       
        self.current_file_label = Text(self.editor_frame,width=70,height=3,relief=FLAT,background=self.editor_frame.cget('bg'), highlightbackground=self.editor_frame.cget('bg'))
        self.current_file_label.pack(side=TOP)
        self.current_file_label.configure(state="disabled")

        # editor box
        self.editor_box = Frame(self.editor_frame)
        self.editor_box.pack(fill=X,side=TOP)
        
        self.editor=Text(self.editor_box,height=30,width=80,background='white')
        self.editor.pack(side=LEFT)
        scroll=Scrollbar(self.editor_box)
        scroll.pack(side=LEFT,fill=Y)
        self.editor.config(yscrollcommand=scroll.set)
        scroll.config(command=self.editor.yview)

        self.editor.bind("<Key>",self.setModified)

        # tags for text highlighting
        self.editor.tag_config("comment", foreground="red", font="Arial 10 italic")

        # insert/remove copyright buttons
        self.copyright_button_frame = Frame(self.editor_frame)
        self.copyright_button_frame.pack(side=TOP)

        self.auto_prepend = IntVar()
        c = Checkbutton(self.copyright_button_frame, text="Auto Insert", variable=self.auto_prepend)
        c.pack(side=TOP, padx=20)
        if (options.auto_prepend):
            c.select()

        self.insert_button = Button(self.copyright_button_frame, text="Insert Copyright (Alt-i)",command=self.insert)
        self.insert_button.pack(side=LEFT, padx=2)
        WINDOW.bind("<Alt-i>", self.insert)

        self.force_insert_button = Button(self.copyright_button_frame, text="Force insert (Alt-f)",command=self.forceInsert)
        self.force_insert_button.pack(side=LEFT, padx=2)
        WINDOW.bind("<Alt-f>", self.forceInsert)

        self.remove_button = Button(self.copyright_button_frame, text="Remove Copyright (Alt-r)",command=self.remove)
        self.remove_button.pack(side=LEFT, padx=2)
        WINDOW.bind("<Alt-r>", self.remove)

        # editor buttons
        self.editor_button_frame = Frame(self.editor_frame)
        self.editor_button_frame.pack(side=TOP)

        self.save_button = Button(self.editor_button_frame, text="Save (Alt-s)",command=self.save)
        self.save_button.pack(side=LEFT, padx=2)
        WINDOW.bind("<Alt-s>", self.save)
        self.complete_button = Button(self.editor_button_frame, text="Complete (Alt-c)",command=self.complete)
        self.complete_button.pack(side=LEFT, padx=2)
        WINDOW.bind("<Alt-c>", self.complete)
        self.mark_button = Button(self.editor_button_frame, text="TODO (Alt-t)",command=self.todo)
        self.mark_button.pack(side=LEFT, padx=2)
        WINDOW.bind("<Alt-t>", self.todo)

        # copyright selection buttons 
        self.choose_copyright_frame = Frame(self.editor_frame)
        self.choose_copyright_frame.pack(side=TOP)
    
        self.choose_copyright_button = Button(self.choose_copyright_frame, text="Choose copyright file",command=self.chooseCopyright)
        self.choose_copyright_button.pack(side=LEFT, padx=40)
        self.default_copyright_button = Button(self.choose_copyright_frame, text="Use Default copyright",command=self.defaultCopyright)
        self.default_copyright_button.pack(side=LEFT, padx=40)

        # comment char frame
        self.comment_selector_frame = Frame(self.editor_frame)
        self.comment_selector_frame.pack(side=TOP)

        self.comment_type = StringVar()
        self.comment_type.set(DEFAULT_COMMENT_CHAR)
        
        comment_options = [DEFAULT_COMMENT_CHAR]

        for f in FORCED_COMMENT_CHARS:
            comment_options.append(f[0])

        Label(self.comment_selector_frame, text="Comment character style:").pack(side=LEFT)
        self.comment_type_menu = OptionMenu(self.comment_selector_frame, self.comment_type, *comment_options)
        self.comment_type_menu.pack(side=LEFT,padx=10)


        self.currentFileIndex = -1

        # load the directory specified in the commandline (if any)
        if (options.directory):
            self.default_dir = options.directory
            self.directory = options.directory
            self.loadDirectory()

        self.setButtonStates()

    """
    Set the states (enabled/disabled) of the editor buttons
    based on the current state of the app
    """
    def setButtonStates(self):
        if self.currentFileIndex < 0:
            self.save_button.config(state = DISABLED)
            self.complete_button.config(state = DISABLED)
            self.mark_button.config(state = DISABLED)
            self.insert_button.config(state = DISABLED)
            self.remove_button.config(state = DISABLED)
            self.force_insert_button.config(state = DISABLED)
        else:
            self.complete_button.config(state = NORMAL)
            self.mark_button.config(state = NORMAL)
            self.insert_button.config(state = NORMAL)
            self.remove_button.config(state = NORMAL)
            self.force_insert_button.config(state = NORMAL)
            if (self.fileModified):
                self.save_button.config(state = NORMAL)
                self.complete_button.config(text ="Save and Complete (Alt-c)")
            else:
                self.save_button.config(state = DISABLED)
                self.complete_button.config(text ="Complete (Alt-c)")

    """
    A calback function to set a file as modified if anything is typed
    into the editor box. Note dummy argument required for callback
    """
    def setModified(self,dummy=None):
        self.fileModified = True
        self.setCurrentFileText()
        self.setButtonStates()
        
    """
    Set the current file text, along with the modified state
    """
    def setCurrentFileText(self):
        filename = ""
        if self.currentFileIndex >= 0:
            if self.fileModified:
                filename = self.files[self.currentFileIndex][1] + "*"
            else:
                filename = self.files[self.currentFileIndex][1]            

        self.current_file_label.configure(state="normal")
        self.current_file_label.delete(1.0, END)
        self.current_file_label.insert(END, filename)
        self.current_file_label.configure(state="disabled")

    """
    Read the current contents of the editor box and update
    editor text variable
    """
    def getEditorLines(self):
        text = self.editor.get("1.0",END)
        self.editor_text=[]
        lines = text.split('\n')

        count = 0
        for l in lines:
            count = count + 1
            if count == len (lines):
                if text.endswith("\n"):
                    self.editor_text.append(l+'\n')
                else:
                    self.editor_text.append(l)
            else:
                self.editor_text.append(l+'\n')

    """
    Save the current editor file to disk and mark it as 'modified'
    """
    def save(self,dummy=0):
        if self.currentFileIndex < 0:
            return

        text = self.editor.get("1.0",END)
        file = open(self.files[self.currentFileIndex][1], "w")
        try:
            text.encode("ascii")
        # handle files that have non-ASCII characters
        except (UnicodeEncodeError ,UnicodeDecodeError):
            text = text.encode("utf-8")

        file.write(text)
        file.close()

        self.files[self.currentFileIndex][2] = "modified"
        self.populateFiles()
        
        self.writeDirectoryConfig()

        self.fileModified = False
        self.setCurrentFileText()
        self.setButtonStates()

        # highlight our file in the filelist, so up/down arrow keys
        # remember where they left off!
        self.filelist.activate(self.currentFileIndex)
        self.filelist.selection_set(first=self.currentFileIndex)

        self.filelist.focus_set()

    """
    Save the editor file and mark it as 'complete'
    """
    def complete(self,dummy=0):
        if self.currentFileIndex < 0:
            return

        # save the file if it's been modified
        if self.fileModified:
            self.save()

        self.files[self.currentFileIndex][2] = "completed"
        self.populateFiles()
        self.writeDirectoryConfig()

        # highlight our file in the filelist, so up/down arrow keys
        # remember where they left off!
        self.filelist.activate(self.currentFileIndex)
        self.filelist.selection_set(first=self.currentFileIndex)

        self.filelist.focus_set()

    """
    Mark the editor file as 'todo'. Note does NOT save!
    """
    def todo(self,dummy=0):
        if self.currentFileIndex < 0:
            return

        self.files[self.currentFileIndex][2] = "todo"
        self.populateFiles()
        self.writeDirectoryConfig()        

        # highlight our file in the filelist, so up/down arrow keys
        # remember where they left off!
        self.filelist.activate(self.currentFileIndex)
        self.filelist.selection_set(first=self.currentFileIndex)

    """
    Insert the current copyright text at the top of the file being edited
    """
    def insert(self,dummy=0):
        if self.fileModified:
            self.getEditorLines()

        if self.prependCopyright():
            self.setModified()
            self.loadEditorText()

        # highlight our file in the filelist, so up/down arrow keys
        # remember where they left off!
        self.filelist.activate(self.currentFileIndex)
        self.filelist.selection_set(first=self.currentFileIndex)

    """
    Force the insert the current copyright text at the top of the file being edited
    """
    def forceInsert(self,dummy=0):
        if self.fileModified:
            self.getEditorLines()

        if self.prependCopyright(True):
            self.setModified()
            self.loadEditorText()

        # highlight our file in the filelist, so up/down arrow keys
        # remember where they left off!
        self.filelist.activate(self.currentFileIndex)
        self.filelist.selection_set(first=self.currentFileIndex)

    """
    Remove an existing copyright
    """
    def remove(self,dummy=0):
        if self.removeCopyright():
            self.setModified()
            self.loadEditorText()

        # highlight our file in the filelist, so up/down arrow keys
        # remember where they left off!
        self.filelist.activate(self.currentFileIndex)
        self.filelist.selection_set(first=self.currentFileIndex)

    """
    Choose a text file containing the desired copyright header
    """
    def chooseCopyright(self):
        dir_opt = options = {}
        options['initialdir'] = self.copyright_file
        options['title'] = 'Choose a copyright text file'

        copyright_file = tkFileDialog.askopenfilename(**dir_opt)

        if copyright_file:
            self.copyright_file = copyright_file

    """
    Use the default copyright from copyright_prepend
    """
    def defaultCopyright(self):
        self.copyright_file = None

    """
    Choose the root of the directory tree to edit
    """
    def chooseDirectory (self):
        dir_opt = {}
        dir_opt['initialdir'] = self.default_dir 
        dir_opt['mustexist'] = True
        dir_opt['title'] = 'Choose a directory to edit'

        directory = tkFileDialog.askdirectory(**dir_opt)
        if not directory:
            return

        self.default_dir = directory
        self.directory = directory

        self.filelist.yview_moveto(0)
        self.loadDirectory()

    """
    Load the selected directory, including the file list and file metadata
    """
    def loadDirectory(self):
        self.files=[]
        
        self.getFiles(self.directory)
        
        self.readDirectoryConfig()
        self.populateFiles()

        # delete editor contents and de-select the current file
        self.editor.delete(1.0, END)
        self.currentFileIndex = -1 
        self.setCurrentFileText()

        self.setButtonStates()
    
    """
    Populate the GUI filelist, and highlight the files based on the current
    markings
    """
    def populateFiles(self):
        # remember our last position
        position = self.filelist.yview()
        self.filelist.delete(0, END)
        i = 0
        count = 0
        for f in self.files:
            self.filelist.insert(END, f[0])

            # add colors to a listbox
            if f[2] == 'modified':
                self.filelist.itemconfig(count, fg=MODIFIED_COLOR)
            if f[2] == 'completed':
                self.filelist.itemconfig(count, fg=COMPLETED_COLOR)
            if f[2] == 'todo':
                self.filelist.itemconfig(count, fg=TODO_COLOR)
            count = count + 1

        # restore the last position so we don't end up back at the
        # top of the list
        self.filelist.yview_moveto(position[0])

    """
    Read file metadata, if a metadata file is present at the root of the current
    directory tree
    """
    def readDirectoryConfig(self):
        try:
            file = open(self.directory+"/"+CONFIG_FILE_NAME)
        except IOError as e:
            return

        while 1:
            line = file.readline()
            if not line:
                break

            # get rid of the newline char
            line = line.rstrip()
            
            toks = line.split(' ')

            for f in self.files:
                if f[1].endswith(toks[0]):
                    f[2] = toks[1]
                    break
       
    """
    Write the file metadata to disk
    """
    def writeDirectoryConfig(self):
        file = open(self.directory+"/"+CONFIG_FILE_NAME, "w")
    
        for f in self.files:
            if f[2] != "unmodified":
                file.write("%s %s\n" % (f[1], f[2]))

        file.close()

    """
    Load the file selected in the file chooser
    """ 
    def selectFile (self, evt):

        self.fileModified = False

        w = evt.widget
        # maybe they clicked in an empty part of the box?
        if not w.curselection():
            return
            
        index = int(w.curselection()[0])
        self.currentFileIndex = index
        value = w.get(index)
        
        # read the file contents into a list of lines
        filename = self.files[index][1]
        file = open(filename)
        self.editor_text=[]

        while 1:
            line = file.readline()
            if not line:
                break
            self.editor_text.append(line)

        # a hack to prevent adding a newline at the end of the
        # file. readline should have taken care of this...
        # TODO some way of doing this without having to reread the
        #      entire file!
        text = file.read()
        if not text.endswith("\n") and len(self.editor_text) > 0:
            line = self.editor_text[len(self.editor_text)-1]
            self.editor_text[len(self.editor_text)-1] = line.rstrip('\n')

        if self.auto_prepend.get():
            # try to add a default copyright
            self.fileModified = self.prependCopyright()

        self.setCurrentFileText()

        # put the text into the editor gui
        self.loadEditorText()

        self.setButtonStates()

    """
    Try to remove a copyright using copyright_remove
    """
    def removeCopyright(self):
        filename = self.files[self.currentFileIndex][0]
        exts = filename.split(".")
        tmpfilename = self.directory+"/tmp."+exts[-1]
    
        # write a temporary file, so we don't inadvertently
        # modify the original without the user's consent
        file = open(tmpfilename,'w');
        for line in self.editor_text:
            file.write(line)
        file.close()

        cmd = ["copyright_remove"]
        if options.keystring_remove:
            cmd.append("-k")
            cmd.append( options.keystring_remove )
        cmd.append(tmpfilename)

        p = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        output, errors = p.communicate()

        if options.debug:
            print output
            print errors

        # did copyright_remove do anything?
        if re.search("copyright not removed",errors):
            os.remove(tmpfilename)    
            return False

        # now reread file, with perhaps the copyright removed
        file = open(tmpfilename)
        self.editor_text = []
        while 1:
            line = file.readline()
            if not line:
                break
            self.editor_text.append(line)

        # delete the temp file
        os.remove(tmpfilename)    

        return True

    """
    Try to prepend a copyright using copyright_prepend
    """
    def prependCopyright(self,force_insert=False):
        filename = self.files[self.currentFileIndex][0]
        exts = filename.split(".")
        tmpfilename = "/tmp/"+filename
    
        # write a temporary file, so we don't inadvertently
        # modify the original without the user's consent
        file = open(tmpfilename,'w');
        for line in self.editor_text:
            try:
                line.encode("ascii")
                # handle files that have non-ASCII characters
            except (UnicodeEncodeError, UnicodeDecodeError):
                line = line.encode("utf-8")

            file.write(line)
        file.close()

        # run copyright_prepend on the temporary file
        cmd = ["copyright_prepend", '-b']
        if options.keystring:
            cmd.append("-k")
            cmd.append( options.keystring )
        
        if self.copyright_file:
            cmd.append("-a")
            cmd.append(self.copyright_file)

        if force_insert:
            cmd.append("-x")

        if not self.comment_type.get() == DEFAULT_COMMENT_CHAR:
            force_option = ""
            for f in FORCED_COMMENT_CHARS:
                if f[0] == self.comment_type.get():
                    force_option = "-" + f[1]
                    break
            cmd.append(force_option)
     
        cmd.append(tmpfilename)

        p = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        output, errors = p.communicate()

        if options.debug:
            print output
            print errors

        # did copyright_prepend do anything?
        if re.search("Skipping ",errors):
            os.remove(tmpfilename)    
            return False

        # now reread file, with perhaps a copyright added
        file = open(tmpfilename)
        self.editor_text = []
        while 1:
            line = file.readline()
            if not line:
                break
            self.editor_text.append(line)

        # delete the temp file
        os.remove(tmpfilename)    
        
        return True

    """
    Load the currently selected file text into the editor, highlighting comment sections
    """
    def loadEditorText(self):
        self.editor.delete(1.0, END)
        count = 0
        highlight = False

        for line in self.editor_text:
            count = count + 1

            # look for single line comments
            if re.search("^\s*[#|!|%|;]", line) or re.search("^--\s", line) or re.search("^\s*C\s\s", line) or re.search("^\s*//",line) or re.search("^\s*\\.\\\\",line):
                self.editor.insert(END, line,("comment"))

            else:

                # look for multiline comments, and turn on/off highlighting
                # mode as appropriate
                if re.search("/\*",line) or re.search("<!--",line):
                    self.editor.insert(END, line,("comment"))
                    highlight = True
                elif (re.search("\*/",line) or re.search("-->",line)) and highlight==True:
                    self.editor.insert(END, line,("comment"))
                    highlight = False
                elif highlight == True:
                    self.editor.insert(END, line,("comment"))
                else:
                    self.editor.insert(END, line)

                # comments might close on the same line they open, so check for this
                if re.search("\*/",line) or re.search("-->",line):
                    highlight = False

    """
    Compile a list of files in the current directory tree, filtering
    on file type and extensions
    """
    def getFiles (self, directory):
        entries = os.listdir(directory)

        for entry in entries:
            if entry.startswith('.'):
                continue

            path = os.path.join(directory, entry)

            # skip symlinks
            if os.path.islink(path):
                continue

            # if it's a directory, recursively look
            # for files to edit in that directory
            if os.path.isdir(path) == True:
                if not entry == "CVS": # don't recurse into cvs directories
                    self.getFiles(path)
                continue

            # skip irregular files
            if not os.path.isfile(path):
                continue

            # see if this file is one that potentially needs
            # a copyright
            include = True
            for ext in EXCLUDED_FILE_EXTENSIONS:
                if entry.endswith("."+ext):
                    include = False
                    break

            if not include:
                continue

            # now see what UNIX 'file' command has to say about the file
            cmd = ['file',path]
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            output, errors = p.communicate()

            include = False

            for filetype in INCLUDED_FILE_TYPES:
                if re.search(filetype, output):
                    include = True
                    break
            
            if not include:
                continue

            self.files.append([entry, path,"unmodified"])

    """
    Parse command line arguments
    """
    def getOptions(self):

        global options

        parser = OptionParser(usage = "%prog [options]")
        parser.add_option("-d", "--directory", dest="directory", default="", help="Start in this directory")
        parser.add_option("-a", "--auto_prepend", dest="auto_prepend", action="store_true", default=False, help="Run copyright_prepend automatically on selected files")
        parser.add_option("--debug", dest="debug", action="store_true", default=False, help="Print debugging messages")
        parser.add_option("-k", "--keystring", dest="keystring", default="",help="Use this character string for the copyright 'keystring' when adding copyrights")
        parser.add_option("-r", "--keystring-remove", dest="keystring_remove", default="", help="Use this character string for the copyright 'keystring' when removing copyrights")

        (options,args) = parser.parse_args()

"""
Entry point of the application
"""
if __name__ == "__main__":
    prog = Program()
    WINDOW.mainloop()
