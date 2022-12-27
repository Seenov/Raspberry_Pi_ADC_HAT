from _support_Beta1 import *
from Files import File 
#from embedded_media.app_logo import APP_LOGO_FILE_LIKE
""" To reduce cluter in the main function of the app, I decided to use one function per component to make sure GUI stuff stays seperated
    and organized as there will be """

# For simplicity I am using place geometry manager as it gives you the freedom to place each widget individually
# and it is easier to tweak in the later stages.

# -> Specific Components of the GUI

def window_logo(parent):
    with open('temp.png', 'wb') as file:
            file.write(APP_LOGO_FILE_LIKE)
    bg_image_label = customtkinter.CTkLabel(master=parent, image=Load_image((dir_path()+ '/temp.png'), 100, 30), text='')
    
    
    os.remove('temp.png')
    return bg_image_label

def LOGO(parent):
    """ Specific Image Label """
    
    #bg_image_label = window_logo(parent=parent)
    bg_image_label = customtkinter.CTkLabel(master=parent, text='SEENOV', font=Heading_font())
    bg_image_label.place(x=15, y=5, width=100, height=50)
    return bg_image_label

def T_LABEL(parent): # Title Label
    """ Label title """
    app_title_label = customtkinter.CTkLabel(master=parent, text= 'ADC Control Panel', font=Heading_font())
    app_title_label.place(x=180, y=6, width=245, height=50)
    return app_title_label

def S_LABEL(parent, text='', x=10, y=0, width=0, height=0): # Styled Label
    """ Label with a specific style that will be used to display information """ #, fg_color='#222222'
    general_app_label = customtkinter.CTkLabel(master=parent, text=text, text_color="#006F20", corner_radius=5)
    general_app_label.place(x=x, y=y, width=width, height=height)
    return general_app_label

# -> General Components of the GUI
def LABEL(parent, text='', x=0, y=0, width=0, height=0):
    """ A general label """
    general_app_label = customtkinter.CTkLabel(master=parent, text=text, font=Heading_font())
    general_app_label.place(x=x, y=y, width=width, height=height)
    return general_app_label

def BUTTON(parent, text='', command=lambda: print("button"), x=0, y=0, width=0, height=0):
    """ A general button """
    general_btn = customtkinter.CTkButton(master=parent, text=text, font=Button_font(), command=command)
    general_btn.place(x=x, y=y, width=width, height=height)
    return general_btn

def SLIDER(parent, x=0, y=0, from_=0, to_=0):
    slider = customtkinter.CTkSlider(master=parent, orientation="vertical", from_=from_, to=to_)
    slider.place(x=x, y=y)
    return slider

def TEXTBOX(parent, x=0, y=0, width=0, height=0, placeholder='Rate is mSec between ADC conversions\nRead Documentation before using OTA'):
    textbox = customtkinter.CTkTextbox(master=parent, width=width, height=height)
    textbox.insert("0.0", placeholder)
    textbox.place(x=x, y=y, width=width, height=height)
    return textbox

def CHECKBOX(parent, text, command=lambda: print("checkbox"), x=0, y=0):
    switch = customtkinter.CTkSwitch(master=parent, command=command, text=text)
    switch.place(x=x, y=y)
    
    return switch

if __name__ == "__main__":
    import sys
    from ADC_UI_Beta1 import main
    main(sys.argv)

