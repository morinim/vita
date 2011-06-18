#!/usr/bin/python

import ConfigParser
import os.path
import subprocess

from vita_sr_gui import *

class VitaSrConfiguration:
    def __init__(self):
        self.program_path = ""
        self.training_data = ""
        self.validation_data = ""

    def load(self, filename):
        config = ConfigParser.SafeConfigParser()
        config.read(filename)

        #float = config.getfloat("Section1", "float")
        #int = config.getint("Section1", "int")
            
        section = "DATA"
        if (config.has_section(section)):
            self.training_data = config.get(section, "Training data")
            self.validation_data = config.get(section, "Validation data")

        section = "OPTIONS"
        if (config.has_section(section)):
            self.program_path = config.get(section, "Program path")
        
        return True


    def save(self, filename):
        # When adding sections or items, add them in the reverse order of
        # how you want them to be displayed in the actual file.
        config = ConfigParser.SafeConfigParser()
        
        print(filename)

        section = "OPTIONS"
        config.add_section(section)
        config.set(section, "Program path", self.program_path)

        section = "DATA"
        config.add_section(section)
        config.set(section, "Validation data", self.validation_data)
        config.set(section, "Training data", self.training_data)

        with open(filename, "wb") as configfile:
            config.write(configfile)

        return True




class VitaSrProcess:
    pass




class VitaSrGui:
    def __init__(self, ui):
        self.config_ = VitaSrConfiguration()
        self.prj_name_ = ""
        self.ui_ = ui

        QtCore.QObject.connect(ui.act_open_project,
                               QtCore.SIGNAL("triggered()"),
                               self.open_project)
        QtCore.QObject.connect(ui.act_save_project,
                               QtCore.SIGNAL("triggered()"),
                               self.save_project)
        QtCore.QObject.connect(ui.act_start,
                               QtCore.SIGNAL("triggered()"),
                               self.start_vita)
        QtCore.QObject.connect(ui.pb_browse_program_path,
                               QtCore.SIGNAL("clicked()"),
                               self.set_program_path)
        QtCore.QObject.connect(ui.pb_browse_training_data,
                               QtCore.SIGNAL("clicked()"),
                               self.set_training_data_file)
        QtCore.QObject.connect(ui.pb_browse_validation_data,
                               QtCore.SIGNAL("clicked()"),
                               self.set_validation_data_file)
        QtCore.QObject.connect(ui.pb_browse_symbols,
                               QtCore.SIGNAL("clicked()"),
                               self.set_symbols_file)

    def get_filename(self):
        dialog = QtGui.QFileDialog()
        dialog.setFileMode(QtGui.QFileDialog.ExistingFile)
        return unicode(dialog.getOpenFileName())


    def open_project(self):
        file_name = self.get_filename()
        if (file_name != "" and self.config_.load(file_name)):
            self.set_project_name(file_name)

            self.ui_.le_program_path.setText(self.config_.program_path)
            self.ui_.le_training_data.setText(self.config_.training_data)
            self.ui_.le_validation_data.setText(self.config_.validation_data)

            return True

        return False


    def save_project(self):
        return self.save_project_with_name(self.prj_name_)


    def save_project_with_name(self, name):
        self.config_.program_path = unicode(self.ui_.le_program_path.text())
        self.config_.training_data = unicode(self.ui_.le_training_data.text())
        self.config_.validation_data = unicode(self.ui_.le_validation_data.text())

        # Writing our configuration file.
        if (name == ""):
            dialog = QtGui.QFileDialog()
            name = dialog.getSaveFileName()

        if (name != "" and self.config_.save(name)):
            self.set_project_name(name)
            return True
        
        return False

        
    def set_file(self, le):
        file_name = self.get_filename()
        if (file_name != ""):
            le.setText(file_name)


    def set_program_path(self):
        self.set_file(self.ui_.le_program_path)


    def set_project_name(self, name):
        self.ui_.WindowTitle = "VITA";
        self.prj_name_ = name
        if (name != ""):
            self.ui_.WindowTitle += " - " + unicode(name)
            #self.ui_.WindowTitle = self.ui_.WindowTitle.join(unicode(name_))

   
    def set_symbols_file(self):
        self.set_file(self.ui_.le_symbols)


    def set_training_data_file(self): 
        self.set_file(self.ui_.le_training_data)


    def set_validation_data_file(self):
        self.set_file(self.ui_.le_validation_data)


    def start_vita(self):
        cmd = os.path.join(self.config_.getui_.le_program_path, "sr")
        args = ""
        _process = Popen([cmd, args])




if __name__ == "__main__":
    import sys

    app = QtGui.QApplication(sys.argv)
    mw = QtGui.QMainWindow()

    ui = Ui_main_window()
    ui.setupUi(mw)

    vita_gui = VitaSrGui(ui)

    mw.show()

    sys.exit(app.exec_())
