#include "appwindow.hpp"
#include <iostream>

AppWindow::AppWindow()
{
  set_title("488 Tetrominoes on the Wall");

  // A utility class for constructing things that go into menus, which
  // we'll set up next.
  using Gtk::Menu_Helpers::MenuElem;
  using Gtk::Menu_Helpers::RadioMenuElem;
  using Gtk::Menu_Helpers::CheckMenuElem;
  
  // Set up the application menu
  // The slot we use here just causes AppWindow::hide() on this,
  // which shuts down the application.
  m_menu_app.items().push_back(MenuElem("_Quit", Gtk::AccelKey("q"),
    sigc::mem_fun(*this, &AppWindow::hide)));

  m_menu_app.items().push_back(MenuElem("_New Game", Gtk::AccelKey("n"),
    sigc::mem_fun(m_viewer, &Viewer::new_game))); //TODO: Implement new_game()
  m_menu_app.items().push_back(MenuElem("_Reset", Gtk::AccelKey("r"),
    sigc::mem_fun(*this, &AppWindow::hide))); //TODO: hide()

  // Set up the draw mode menu
  sigc::slot1<void, Viewer::ColorMode> color_mode_slot =
      sigc::mem_fun(m_viewer, &Viewer::set_color_mode);

  m_menu_drawmode.items().push_back(RadioMenuElem(m_draw_group, "_Wire_frame", Gtk::AccelKey("w"),
    sigc::bind( color_mode_slot, Viewer::WIRE_FRAME))); //TODO
  m_menu_drawmode.items().push_back(RadioMenuElem(m_draw_group, "_Face", Gtk::AccelKey("f"),
    sigc::bind( color_mode_slot, Viewer::FACE))); //TODO
  m_menu_drawmode.items().push_back(RadioMenuElem(m_draw_group, "_Multicoloured", Gtk::AccelKey("m"),
    sigc::bind( color_mode_slot, Viewer::MULTICOLOR))); //TODO

  // Set up the speed mode menu
  m_menu_speed.items().push_back(RadioMenuElem(m_speed_group, "_Slow", Gtk::AccelKey("1"),
    sigc::mem_fun(*this, &AppWindow::hide))); //TODO
  m_menu_speed.items().push_back(RadioMenuElem(m_speed_group, "_Medium", Gtk::AccelKey("2"),
    sigc::mem_fun(*this, &AppWindow::hide))); //TODO
  m_menu_speed.items().push_back(RadioMenuElem(m_speed_group, "_Fast", Gtk::AccelKey("3"),
    sigc::mem_fun(*this, &AppWindow::hide))); //TODO

  // Set up the buffering menu
  m_menu_buffering.items().push_back(CheckMenuElem("_Double Buffering", Gtk::AccelKey("b"),
    sigc::mem_fun(*this, &AppWindow::hide))); //TODO

  // Set up the menu bar
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Application", m_menu_app));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Draw Mode", m_menu_drawmode));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Speed", m_menu_speed));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Buffering", m_menu_buffering));

  // Pack in our widgets
  
  // First add the vertical box as our single "top" widget
  add(m_vbox);

  // Put the menubar on the top, and make it as small as possible
  m_vbox.pack_start(m_menubar, Gtk::PACK_SHRINK);

  // Put the viewer below the menubar. pack_start "grows" the widget
  // by default, so it'll take up the rest of the window.
  m_viewer.set_size_request(300, 600);
  m_vbox.pack_start(m_viewer);
  m_game = m_viewer.get_game();

  show_all();
}

bool AppWindow::on_key_press_event( GdkEventKey *ev )
{
        // This is a convenient place to handle non-shortcut
        // keys.  You'll want to look at ev->keyval.

	// An example key; delete and replace with the
	// keys you want to process
  std::cerr << "keyval: " << ev->keyval << std::endl;
        if( ev->keyval == 't' ) {
                std::cerr << "Hello!" << std::endl;
//                return true;
        }
        else if ( ev->keyval == GDK_KEY_Shift_L ||
                  ev->keyval == GDK_KEY_Shift_R )
        {
                m_viewer.shift_set(true);
        }
        else if ( ev->keyval == GDK_Left )
	{
                m_game->moveLeft();
        }
        else if ( ev->keyval == GDK_Up )
	{
                m_game->rotateCW();
        }
        else if ( ev->keyval == GDK_Right )
	{
                m_game->moveRight();
        }
        else if ( ev->keyval == GDK_Down)
	{
                m_game->rotateCCW();
        }
        else if ( ev->keyval == GDK_space )
	{
                m_game->drop();
        }
        else {
                return Gtk::Window::on_key_press_event( ev );
        }
        return true;
}

bool AppWindow::on_key_release_event( GdkEventKey *ev )
{
        if ( ev->keyval == GDK_KEY_Shift_L ||
             ev->keyval == GDK_KEY_Shift_R )
        {
                m_viewer.shift_set(false);
                return true;
        }
        return true;
}
