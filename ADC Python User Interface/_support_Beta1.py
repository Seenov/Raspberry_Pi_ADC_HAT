import customtkinter
import os
""""
sudo apt-get install python3-pil.imagetk
"""


#from embedded_media.Files import File 
#from embedded_media.app_icon import APP_ICON_FILE_LIKE
# Some files are not used. They will be required when we fix the
# Seenov logo problem

def app_setup(ctk_instance):
    """ Initial Setup of the Application takes in the root and configures it """
    width = 660
    height =840
    screenwidth = ctk_instance.winfo_screenwidth()
    screenheight = ctk_instance.winfo_screenheight()
    # Below proper alignment is calculated so that every time on any screen app starts in the middle
    alignstr = '%dx%d+%d+%d' % (width, height, (screenwidth - width) / 2, (screenheight - height) / 2)
    ctk_instance.geometry(alignstr)
    #
    
    #window_icon(ctk_instance=ctk_instance)
    ctk_instance.resizable(width=True, height=True)
    ctk_instance.title("SEENOV ESP32C3 ADC UI")

def window_icon(ctk_instance):
    """ the tkinter wasn't loading directly from a file-like object
        I used a temporary file to read it everytime the app is started
        it won't do any errors and icons and images will be embedded """
    with open('temp.ico', 'wb') as file:
        file.write(APP_ICON_FILE_LIKE)
    ctk_instance.iconbitmap(default="temp.ico")
    try:
        os.remove('temp.ico')
    except Exception:
        pass

def dir_path():
    """ Returns path of the current working directory """
    return os.path.dirname(os.path.realpath(__file__))

def Load_image(path_to_image, width=None, height=None, direct=False):
    """ Returns back a CTKImage object """
    if width and height != None:
        loaded_image = customtkinter.CTkImage(Image.open(path_to_image), size=(width, height))
    elif direct == True:
        loaded_image = customtkinter.CTkImage(path_to_image)
    else:
        loaded_image = customtkinter.CTkImage(Image.open(path_to_image))
    return loaded_image

def Heading_font():
    """ Usage like this makes it easier to edit the font later when components use value being returned from Heading_font()"""
    return ('Ariel Bold', 20) 

def Button_font():
    return ('Ariel', 20) 

if __name__ == "__main__":
    from ADC_UI_Beta1 import main
    main()
