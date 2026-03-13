#include <gtk/gtk.h>
#include <gtkmm.h>
#include <X11/Xlib.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <filesystem>

using namespace std;

void InstallAPK(const std::string& file, Gtk::Label* errors){
    std::string cmd = "waydroid app install \"" + file + "\"";
    int result = system(cmd.c_str());

    if (result != 0){
        errors->set_text("Hiba történt!\n");
    }
    else{
        errors->set_text("Telepítés kész!!.\n");
    }
}

bool weston_run(){
    return system("weston status | grep -q RUNNING") == 0;
}

bool waydroid_run(){
    return system("waydroid status | grep -q RUNNING") == 0;
}

std::string IPC_Comm(){
    const char* runtimeDir = getenv("XDG_RUNTIME_DIR");  // Lekérjük a XDG_RUNTIME_DIR könyvtárat

    if (!runtimeDir) {                                  // Ha nincs beállítva, térjünk vissza a "wayland-0"-val
        return "wayland-0";
    }

    for (const auto& entry : std::filesystem::directory_iterator(runtimeDir)) {  // Menjünk végig a könyvtár összes fájlján/könyvtárán
        std::string name = entry.path().filename().string();                    // Csak a fájl nevét nézzük, elérési út nélkül. (std::string charakter tömb)
        if (name.size() >= 8 && name.substr(0, 8) == "wayland-") {             // Ha a név "wayland-"-nal kezdődik, ez egy socket
            return name;                                                      // visszaadjuk az első találatot
        }
    }
    return "wayland-0";    // Ha nincs találat, fallback
}

void startweston(int W,int H){  //folyamatok külön indítáas
    if (fork() == 0){
             execlp("weston",
                    "weston",
                    ("--width=" + std::to_string(W)).c_str(),
                    ("--height=" + std::to_string(H)).c_str(),
                    (char*)nullptr);
             _exit(1);
         }

         sleep(3);

         if (fork() == 0){
             setenv("WAYLAND_DISPLAY", IPC_Comm().c_str(), 1); // ipc_Comm vissza térését .c_str() char ra konvertál
             execlp("waydroid",
                    "waydroid",
                    "show-full-ui",
                    (char*)nullptr);
             _exit(1);
         }
}

void screen(Gtk::Label* labelWidth, Gtk::Label* labelHeight, Gtk::Button* button, Gtk::Label* errors ){  //label hez hivatkozik a paraméter
    Display* display = XOpenDisplay(NULL);   // Kapcsolódás az X szerverhez
    if (!display) {
        if(errors){
            errors->set_text("Error X11 server!");
        }
        return;
    }

    int screen = DefaultScreen(display);      // Alapértelmezett képernyõ lekérése
    int width = DisplayWidth(display, screen);
    int height = DisplayHeight(display, screen);
    XCloseDisplay(display);   // Kapcsolat bezárása

    if(labelWidth){
        labelWidth->set_text(std::to_string(width));  //lekért méretet át adjuk a labelnek gtk ba
    }

    if(labelHeight){
        labelHeight->set_text(std::to_string(height));
    }
    if (button) {
            button->signal_clicked().connect(
                [width, height, errors]() {               //capture list  külső változók átvétele labdákba
                    if(waydroid_run() == false){         // ha nem fut elinditja (ha nem az eseményen belül vizsgáljuk mindig tru értéket kap)
                        startweston(width, height);     //fügvény hívás eseményre kötve!!
                    }
                    else{
                        errors->set_text("Restart program!");
                    }
                }
                );
    }
}

int main(int argc, char* argv[]){
    auto app = Gtk::Application::create(argc, argv);     //gtkmm-es (C++ wrapper) kód
    auto builder = Gtk::Builder::create_from_file("tesztgtk2.glade");

    Gtk::FileChooserButton* ApkOpen = nullptr;  // fájl kiválasztó GTK létrehozása
    Gtk::Window* window = nullptr;
    Gtk::Window* Infwindow = nullptr;
    Gtk::Button* button = nullptr; 
    Gtk::Button* ApkButton = nullptr;
    Gtk::Button* Infobutton = nullptr;
    Gtk::Label* labelWidth = nullptr;  // ← JAVÍTVA
    Gtk::Label* labelHeight = nullptr;  // ← JAVÍTVA
    Gtk::Label* errors = nullptr;

    builder->get_widget("window1", window);
    builder->get_widget("popup", Infwindow);
     builder->get_widget("ApkOpen", ApkOpen); //ez a fájl tallozó azonosító házzá adása
    builder->get_widget("Apkinstall", ApkButton);
    builder->get_widget("button1", button);
    builder->get_widget("infok", Infobutton);
    builder->get_widget("width", labelWidth);  // gtk label cimke azonosító  "width"
    builder->get_widget("height", labelHeight);
    builder->get_widget("error", errors);
    screen(labelWidth,labelHeight,button,errors);  //képernyő adatok

    if(waydroid_run() == true){
        system("waydroid session stop");
        system("weston session stop");
        errors->set_text("Waydroid Stop!");
    }

    if (ApkButton && ApkOpen && errors)  //Widget (gomb,label) létezik-e a memóriában
    {
        ApkButton->signal_clicked().connect(
            [ApkOpen, errors]()
            {
                std::string file = ApkOpen->get_filename();  //fájl név vizsgálat

                if (file.empty()){  //Választott-e a user fájlt
                    errors->set_text("Nincs fájl kiválasztva!");
                    return;
                }
                if(waydroid_run() == true){  //== true nem kéne ki irni mert alapból true

                    if (file.size() >= 4 && file.substr(file.size() - 4) == ".apk"){  // fájl karakter száma kissebb mint 4 akkor nem .apk
                        InstallAPK(file, errors);
                    }
                    else{
                        errors->set_text("Hiba! Nem APK fájl!");
                    }
                }
                else{
                    errors->set_text("Hiba! Waydroid nem fut!");
                }
            }
            );
    }

    if (Infobutton && Infwindow){
        Infobutton->signal_clicked().connect(
            [Infwindow]()
            {
                Infwindow->show_all();
            }
            );
    }

    return app->run(*window);
}
