import tkinter as tk
from tkinter import Label, Entry
from PIL import Image, ImageTk
import ttkbootstrap as tb
import socket
from threading import Thread


# Variable globale pour stocker le socket client
client_socket = None

def threaded_network_connection():
    thread = Thread(target=start_server)
    thread.start()

    

def send_message(action_number):
    global client_socket
    if client_socket is not None:
        try:
            client_socket.sendall(str(action_number).encode('utf-8'))
            print("message sent {action_number}")
        except Exception as e:
            print(f"Erreur lors de l'envoi du message : {e}")

def send_message_th(message):
    global client_socket
    if client_socket is not None:
        try:
            print(message)  # Afficher le message à envoyer
            # Encodage correct et envoi du message complet
            encoded_message = message.encode('utf-8')
            bytes_sent = client_socket.send(encoded_message)  # Envoi des données
            print(f"{bytes_sent} bytes envoyés")  # Afficher le nombre de bytes envoyés
        except Exception as e:
            print(f"Erreur lors de l'envoi du message : {e}")
            
def start_server():
    global client_socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('127.0.0.1', 6666)
    server_socket.bind(server_address)
    server_socket.listen(1)
    print("Le serveur est démarré et écoute sur:", server_address)

    while True:
        print("En attente de connexion client...")
        client_socket, client_address = server_socket.accept()
        print(f"Connexion de {client_address}")

    # try:
        # Envoi d'un message
    #    message = 'Bonjour, serveur!'
    #    client_socket.sendall(message.encode())
        
        # Recevoir la réponse du serveur (1024 octets max)
    #    response = client_socket.recv(1024)
    #    print('Réponse du serveur:', response.decode())
    #finally:
        # Fermeture du socket
    #    client_socket.close()
    #my_thread.stop()
    #my_thread.join()
def display_content(event=None):
    # Récupérer le texte de l'Entry
    user_input = chat_entry.get()
    send_message_th(user_input)
    # Insérer le texte dans le Text widget à la fin du contenu existant
    chat_text.config(state='normal')  # Assurez-vous que le widget Text est modifiable
    chat_text.insert(tk.END, "[vous] : "+user_input + "\n")  # Ajoute un retour à la ligne après chaque entrée
    chat_text.config(state='disabled')  # Désactiver l'édition pour éviter que l'utilisateur modifie directement le contenu
    
    # Effacer l'Entry pour la prochaine entrée
    chat_entry.delete(0, tk.END)
    #

def display_content_th(message):
    # Assurez-vous que le widget Text est modifiable
    chat_text.config(state='normal')
    chat_text.insert(tk.END, message + "\n")
    chat_text.config(state='disabled')
    chat_text.see(tk.END)  # Défilement automatique vers le bas

def threaded_recption():
    thread = Thread(target=receive_messages)
    thread.start()

def receive_messages():
    while True:
        try:
            mesg = client_socket.recv(256).decode()
            if mesg:
                # Mise à jour de l'interface utilisateur dans le thread principal
                chat_text.after(1, display_content_th, mesg)
        except OSError:  # Peut être déclenché si le socket est fermé
            break

def chat():
    image_label.grid_remove()
    message_label.grid_remove()
    signIn_frame.grid_remove()
    signUp_frame.grid_remove()
    signup_btn.grid_remove()
    signin_btn.grid_remove()
    chat_frame.grid(row=4,column=0,columnspan=3,pady=10,sticky="sew")
    threaded_recption()

# Fonction du button signIn   
def affiche_signIn():
  signUp_frame.grid_remove()
  signup_btn.grid_remove()
  signin_btn.grid_remove()
  quit_btn.grid_remove()
  message_label.grid_remove()
  signIn_frame.grid(row=2,columnspan=3,pady=20)
  chat_frame.grid_remove()
  send_message(1)

# Fonction du button singUp
def affiche_signUp():
  signIn_frame.grid_remove()
  quit_btn.grid_remove()
  signup_btn.grid_remove()
  signin_btn.grid_remove()
  message_label.grid_remove()
  signUp_frame.grid(row=2,columnspan=3,pady=20)
  chat_frame.grid_remove()
  send_message(2)

# Message d'erreur de formule   
def afficher_message_labelin():
    messagein.grid(row=2 , columnspan=2,pady=(10))
    vide_msgin.grid_forget()
    en_lignein.grid_forget()
    not_existin.grid_forget()
    successin.grid_remove()

# Message d'erreur de formule
def afficher_notexist_labelin():
    not_existin.grid(row=2 , columnspan=2,pady=(10))
    vide_msgin.grid_forget()
    not_existin.grid_forget()
    messagein.grid_forget()
    successin.grid_remove()
# Message d'erreur de formule
def afficher_success_labelin():
    successin.grid(row=2 , columnspan=2,pady=(10))
    not_existin.grid_forget()
    vide_msgin.grid_forget()
    not_existin.grid_forget()
    messagein.grid_forget()

# Message d'erreur de formule
def afficher_enligne_labelin():
    en_lignein.grid(row=2 , columnspan=2,pady=(10))
    messagein.grid_remove()
    vide_msgin.grid_forget()
    not_existin.grid_forget()
    successin.grid_remove()

# Message d'erreur de formule
def afficher_vide_labelin():
    vide_msgin.grid(row=2 , columnspan=2,pady=(10))
    messagein.grid_remove()
    en_lignein.grid_forget()
    not_existin.grid_forget()
    successin.grid_remove()

# Fonction du button quit 
def quit_func():
  send_message(3)
  window.quit

# Fonction du submitin
def submitin():
    contenu_user = user_entryin.get()
    contenu_pass = pass_entryin.get()

    if (not contenu_pass or not contenu_user):
        afficher_vide_labelin()
        return None  
    else :
        if(contenu_pass   and (";" in contenu_user  )):
            afficher_message_labelin()
            return None

    
    try:
        # Écrire le nom d'utilisateur dans le socket
        send_message(contenu_user)
        # Lire le drapeau à partir du tube nommé
        flag = client_socket.recv(256).decode()  # Recevoir des données du client (taille du buffer en octets)
        if not flag:
            # Aucune donnée reçue, la connexion peut être fermée
            print("La connexion a été fermée par le client.")
            return None
        print(f"Données reçues: {flag}")
     
        print(flag)
        if flag == "||":
            afficher_enligne_labelin()
            return None
        if flag == "0":
          #Écrire le mot de passe dans le tube nommé
            afficher_notexist_labelin()
            return None
        if flag== "1":
          send_message(contenu_pass)
          afficher_success_labelin()
          print("success")
          chat()
    except Exception as e:
        print(f"Erreur {e}")       
          


# Message d'erreur formule
def afficher_message_labelup():
    messageup.grid(row=2 , columnspan=2,pady=(10))
    vide_msgup.grid_remove()
    en_ligneup.grid_remove()
    existup.grid_remove()
    successup.grid_remove()


# Message d'erreur formule
def afficher_exist_labelup():
    vide_msgup.grid_remove()
    messageup.grid_remove()
    en_ligneup.grid_remove()
    existup.grid(row=2 , columnspan=2,pady=(10))
    successup.grid_remove()

def afficher_vide_labelup():
    vide_msgup.grid(row=2 , columnspan=2,pady=(10))
    messageup.grid_remove()
    en_ligneup.grid_forget()
    existup.grid_forget()
    successup.grid_remove()
# Fonction du submitup
def submitup():
    contenu_user = user_entryup.get()
    contenu_pass = pass_entryup.get()

    if (not contenu_pass or not contenu_user):
        afficher_vide_labelup()
        return None  
    else :
        if(contenu_pass   and (";" in contenu_user  )):
            afficher_message_labelup()
            return None

    
    try:
        # Écrire le nom d'utilisateur dans le socket
        send_message(contenu_user)
        # Lire le drapeau à partir du tube nommé
        flag = client_socket.recv(256).decode()  # Recevoir des données du client (taille du buffer en octets)
        if not flag:
            # Aucune donnée reçue, la connexion peut être fermée
            print("La connexion a été fermée par le client.")
            return None
        print(f"Données reçues: {flag}")
     
        print(flag)
        if flag == "0":
            afficher_exist_labelup()
            return None
     
        if flag== "1":
          send_message(contenu_pass)
          print("success")
          chat()
    except Exception as e:
        print(f"Erreur {e}")      
    






# ----------------Main--------------

window =tb.Window(themename="darkly")
window.title("PHOENIX")
try:
        # Chargement de l'image avec PIL
        icon = Image.open('phoenix.png')
        photo_icon = ImageTk.PhotoImage(icon)
        window.iconphoto(False, photo_icon)
except Exception as e:
        print(f"Erreur lors de la définition de l'icône: {e}")


        # Chargement de l'image avec PIL
window.geometry('620x500')
front_img = Image.open('phoenix_.png')
target_width = 230
ratio = target_width / front_img.width
target_height = int(front_img.height * ratio)
resized_img = front_img.resize((target_width, target_height), Image.Resampling.LANCZOS) 
front_photo = ImageTk.PhotoImage(resized_img)
threaded_network_connection()


# Création et placement des boutons dans un cadre
button_frame = tb.Frame(window)
button_frame.grid(row=0, column=2, sticky='en', padx=(10, 20), pady=(10, 0))
#placement de l'image
image_label=Label(window,image=front_photo)
image_label.grid(row=1, column=0, columnspan=3, pady=(10, 0), sticky='ew')
# Bouton 'Sign Up'
signup_btn = tb.Button(button_frame, text="Sign Up", bootstyle="light outline",command=affiche_signUp)
signup_btn.grid(row=0, column=0, padx=10)

#Bouon 'Sign In'
signin_btn = tb.Button(button_frame, text="Sign In", bootstyle="light outline",command=affiche_signIn)
signin_btn.grid(row=0, column=1, padx=10)


# Bouton 'Quit'
quit_btn = tb.Button(button_frame, text="Quit", bootstyle="danger outline", command=quit_func)
quit_btn.grid(row=0, column=2, padx=10)

# creation de la label frame 
message_label=tb.Label(window,text="PHOENIX",font=("Arial",20),bootstyle="light")
message_label.grid(row=3,columnspan=3,pady=20)


# creation et placement de l'entry de username et passeword et le submit btn de sign in
signIn_frame=tb.Frame(window)
#signIn_frame.grid(row=2,columnspan=3,pady=20)
user_labelin=tb.Label(signIn_frame,text="Username",bootstyle="light")
user_labelin.grid(row=0,column=0,padx=10,pady=10)
user_entryin=tb.Entry(signIn_frame,bootstyle="darkly")
user_entryin.grid(row=0,column=1,padx=10,pady=10)

pass_labelin=tb.Label(signIn_frame,text="Passeword",bootstyle="light")
pass_labelin.grid(row=1,column=0,padx=10,pady=(0,10))
pass_entryin=tb.Entry(signIn_frame,bootstyle="darkly",show="*")
pass_entryin.grid(row=1,column=1,padx=10,pady=(0,10))
vide_msgin=tb.Label(signIn_frame,text="Veuillez remplir tout les champs",bootstyle="danger" ,font=('Arial', 10))
messagein=tb.Label(signIn_frame,text="Ce username exist deja ou bien il contient une ;",bootstyle="danger" ,font=('Arial', 10))
not_existin=tb.Label(signIn_frame,text="Ce username n'exist pas",bootstyle="danger" ,font=('Arial', 10))
en_lignein=tb.Label(signIn_frame,text="Ce username est deja en ligne",bootstyle="danger" ,font=('Arial', 10))
successin=tb.Label(signIn_frame,text="success",bootstyle="danger" ,font=('Arial', 10))
submit_btnin = tb.Button(signIn_frame, text="Submit in", bootstyle="light outline",command=submitin)
submit_btnin.grid(row=3, columnspan=2,pady=(10))



# creation et placement de l'entry de username et passeword et le submit btn de sign up
signUp_frame=tb.Frame(window)
#signUp_frame.grid(row=2,columnspan=3,pady=20)
user_labelup=tb.Label(signUp_frame,text="Username",bootstyle="light")
user_labelup.grid(row=0,column=0,padx=10,pady=10)
user_entryup=tb.Entry(signUp_frame,bootstyle="darkly")
user_entryup.grid(row=0,column=1,padx=10,pady=10)
pass_labelup=tb.Label(signUp_frame,text="Passeword",bootstyle="light")
pass_labelup.grid(row=1,column=0,padx=10,pady=(0,10))
pass_entryup=tb.Entry(signUp_frame,bootstyle="darkly")
pass_entryup.grid(row=1,column=1,padx=10,pady=(0,10))
vide_msgup=tb.Label(signIn_frame,text="Veuillez remplir tout les champs",bootstyle="danger" ,font=('Arial', 10))
messageup=tb.Label(signIn_frame,text="Ce username contient une ;",bootstyle="danger" ,font=('Arial', 10))
existup=tb.Label(signIn_frame,text="Ce username existe deja",bootstyle="danger" ,font=('Arial', 10))
en_ligneup=tb.Label(signIn_frame,text="Ce username est deja en ligne",bootstyle="danger" ,font=('Arial', 10))
successup=tb.Label(signIn_frame,text="success",bootstyle="danger" ,font=('Arial', 10))

submit_btnup = tb.Button(signUp_frame, text="Submit up", bootstyle="light outline",command=submitup)
submit_btnup.grid(row=2, columnspan=2,pady=(20))

# chat box
chat_frame=tb.Frame(window)
chat_frame.grid_rowconfigure(0, weight=1)  # Permet à la ligne du chat_text de s'étendre
chat_frame.grid_rowconfigure(1, weight=0)  # Garde la ligne du entry_button_frame sans expansion
chat_frame.grid_columnconfigure(0, weight=1) 
#chat_frame.grid(row=4,column=0,columnspan=3,pady=10,sticky="sew")
chat_scrol=tb.Scrollbar(chat_frame,orient="vertical",bootstyle="light")
chat_scrol.grid(row=0,column=3,pady=10,sticky="nse")
chat_text=tb.Text(chat_frame,yscrollcommand=chat_scrol.set,wrap="none",font=("Arial",11))
chat_text.grid(row=0,column=0,columnspan=2,pady=10,padx=10,sticky='ew')

entry_button_frame=tb.Frame(chat_frame)
entry_button_frame.grid_rowconfigure(0, weight=1)  # Permet à la ligne du chat_text de s'étendre
entry_button_frame.grid_columnconfigure(0, weight=1) 
 
entry_button_frame.grid_columnconfigure(2, weight=0) 
entry_button_frame.grid(row=1,columnspan=3,sticky='ew')
chat_entry=tb.Entry(entry_button_frame,bootstyle="darkly",width=60,font=("Arial",11))
chat_entry.bind("<Return>", display_content)
chat_button=tb.Button(entry_button_frame, text="Send", bootstyle="light outline",command=display_content)#,command=send
chat_button.grid(row=0, column=2,sticky='es')
chat_entry.grid(row=0,column=0,padx=10,sticky="sew")



# Configuration des poids de colonnes pour permettre l'expansion centrée de l'image
window.grid_columnconfigure(0, weight=1)
window.grid_columnconfigure(1, weight=1)
window.grid_columnconfigure(2, weight=1)
window.grid_rowconfigure(0, weight=0)
window.grid_rowconfigure(1, weight=1)
window.grid_rowconfigure(2, weight=1)
window.grid_rowconfigure(3, weight=1)
window.grid_rowconfigure(4, weight=1)


window.mainloop()
