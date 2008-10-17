/* vi:set sw=2 sts=2 ts=2 et ai tw=100: */
/*-
 * Copyright (c) 2008 Stephan Arts <stephan@xfce.org>
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <glade/glade.h>

#include <libxfce4util/libxfce4util.h>
#include <xfconf/xfconf.h>

#include "xfwm4-dialog_glade.h"
#include "xfwm4-settings.h"

#include "frap-shortcuts-provider.h"



#define DEFAULT_THEME  "Default"

#define INDICATOR_SIZE 9



#define XFWM_SETTINGS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                                     XFWM_TYPE_SETTINGS, \
                                                                     XfwmSettingsPrivate))



typedef struct _MenuTemplate MenuTemplate;



/* Property identifiers */
enum
{
  PROP_0,
  PROP_GLADE_XML,
};



static void       xfwm_settings_class_init                           (XfwmSettingsClass *klass);
static void       xfwm_settings_init                                 (XfwmSettings      *settings);
static void       xfwm_settings_constructed                          (GObject           *object);
static void       xfwm_settings_finalize                             (GObject           *object);
static void       xfwm_settings_get_property                         (GObject           *object,
                                                                      guint              prop_id,
                                                                      GValue            *value,
                                                                      GParamSpec        *pspec);
static void       xfwm_settings_set_property                         (GObject           *object,
                                                                      guint              prop_id,
                                                                      const GValue      *value,
                                                                      GParamSpec        *pspec);
static gint       xfwm_settings_theme_sort_func                      (GtkTreeModel      *model,
                                                                      GtkTreeIter       *iter1,
                                                                      GtkTreeIter       *iter2);
static void       xfwm_settings_load_themes                          (XfwmSettings      *settings);
static void       xfwm_settings_theme_selection_changed              (GtkTreeSelection  *selection,
                                                                      XfwmSettings      *settings);
static void       xfwm_settings_title_alignment_changed              (GtkComboBox       *combo,
                                                                      XfwmSettings      *settings);
static void       xfwm_settings_active_frame_drag_data               (GtkWidget         *widget,
                                                                      GdkDragContext    *drag_context,
                                                                      gint               x,
                                                                      gint               y,
                                                                      GtkSelectionData  *data,
                                                                      guint              info,
                                                                      guint              time,
                                                                      XfwmSettings      *settings);
static gboolean   xfwm_settings_active_frame_drag_motion             (GtkWidget         *widget, 
                                                                      GdkDragContext    *drag_context, 
                                                                      gint               x,
                                                                      gint               y, 
                                                                      guint              time,
                                                                      XfwmSettings      *settings);
static void       xfwm_settings_active_frame_drag_leave              (GtkWidget         *widget,
                                                                      GdkDragContext    *drag_context,
                                                                      guint              time,
                                                                      XfwmSettings      *settings);
static void       xfwm_settings_hidden_frame_drag_data               (GtkWidget         *widget,
                                                                      GdkDragContext    *drag_context,
                                                                      gint               x,
                                                                      gint               y,
                                                                      GtkSelectionData  *data,
                                                                      guint              info,
                                                                      guint              time,
                                                                      XfwmSettings      *settings);
static void       xfwm_settings_delete_indicator                     (GtkWidget         *widget);
static void       xfwm_settings_create_indicator                     (GtkWidget         *widget,
                                                                      gint               x,
                                                                      gint               y,
                                                                      gint               width,
                                                                      gint               height);
static void       xfwm_settings_title_button_drag_data               (GtkWidget        *widget,
                                                                      GdkDragContext   *drag_context,
                                                                      GtkSelectionData *data,
                                                                      guint             info,
                                                                      guint             time,
                                                                      const gchar      *atom_name,
                                                                      XfwmSettings     *settings);
static void       xfwm_settings_title_button_drag_begin              (GtkWidget        *widget,
                                                                      GdkDragContext   *drag_context);
static void       xfwm_settings_title_button_drag_end                (GtkWidget        *widget,
                                                                      GdkDragContext   *drag_context);
static gboolean   xfwm_settings_signal_blocker                       (GtkWidget        *widget);
static GdkPixbuf *xfwm_settings_create_icon_from_widget              (GtkWidget        *widget);

static void       xfwm_settings_save_button_layout                   (XfwmSettings      *settings,
                                                                      GtkContainer      *container);
static void       xfwm_settings_double_click_action_changed          (GtkComboBox       *combo,
                                                                      XfwmSettings      *settings);
static void       xfwm_settings_title_button_alignment_changed       (GtkComboBox       *combo,
                                                                      GtkWidget         *button);

static void       xfwm_settings_button_layout_property_changed       (XfconfChannel     *channel,
                                                                      const gchar       *property,
                                                                      const GValue      *value,
                                                                      XfwmSettings      *settings);
static void       xfwm_settings_title_alignment_property_changed     (XfconfChannel     *channel,
                                                                      const gchar       *property,
                                                                      const GValue      *value,
                                                                      XfwmSettings      *settings);
static void       xfwm_settings_double_click_action_property_changed (XfconfChannel     *channel,
                                                                      const gchar       *property,
                                                                      const GValue      *value,
                                                                      XfwmSettings      *settings);



struct _XfwmSettingsPrivate
{
  GladeXML              *glade_xml;
  FrapShortcutsProvider *provider;
  XfconfChannel         *wm_channel;
};

struct _MenuTemplate
{
  const gchar *name;
  const gchar *value;
};



static GObjectClass      *xfwm_settings_parent_class = NULL;

static const MenuTemplate double_click_values[] = {
  { N_("Shade window"), "shade" },
  { N_("Hide window"), "hide" },
  { N_("Maximize window"), "maximize" },
  { N_("Fill window"), "fill" },
  { N_("Nothing"), "none" },
  { NULL, NULL }
};

static const MenuTemplate title_align_values[] = {
  { N_("Left"), "left" },
  { N_("Center"), "center" },
  { N_("Right"), "right" },
  { NULL, NULL },
};

static gboolean           opt_version = FALSE;
static GdkNativeWindow    opt_socket_id = 0;
static GOptionEntry       opt_entries[] = {
  { "socket-id", 's', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_INT, &opt_socket_id, 
    N_("Settings manager socket"), N_("SOCKET ID") },
  { "version", 'V', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &opt_version, 
    N_("Version information"), NULL },
  { NULL }
};


GType
xfwm_settings_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfwmSettingsClass),
        NULL,
        NULL,
        (GClassInitFunc) xfwm_settings_class_init,
        NULL,
        NULL,
        sizeof (XfwmSettings),
        0,
        (GInstanceInitFunc) xfwm_settings_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfwmSettings", &info, 0);
    }

  return type;
}


/*
 * Xfce 4.6 depends on glib 2.12,
 * Glib 2.14 comes with g_hash_table_get_keys(),
 * until then... use the following function with
 * g_hash_table_foreach()
 */
#if !GLIB_CHECK_VERSION (2,14,0)
static void
xfwm4_settings_get_list_keys_foreach (gpointer key,
                                      gpointer value,
                                      gpointer user_data)
{
  GList **keys = user_data;
  *keys = g_list_prepend (*keys, key);
}
#endif




static void
xfwm_settings_class_init (XfwmSettingsClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (XfwmSettingsPrivate));

  /* Determine the parent type class */
  xfwm_settings_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
#if GLIB_CHECK_VERSION (2,14,0)
  gobject_class->constructed = xfwm_settings_constructed;
#endif
  gobject_class->finalize = xfwm_settings_finalize; 
  gobject_class->get_property = xfwm_settings_get_property;
  gobject_class->set_property = xfwm_settings_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_GLADE_XML,
                                   g_param_spec_object ("glade-xml",
                                                        "glade-xml",
                                                        "glade-xml",
                                                        GLADE_TYPE_XML,
                                                        G_PARAM_CONSTRUCT_ONLY | 
                                                        G_PARAM_WRITABLE));
}



static void
xfwm_settings_init (XfwmSettings *settings)
{
  settings->priv = XFWM_SETTINGS_GET_PRIVATE (settings);

  settings->priv->glade_xml = NULL;
  settings->priv->provider = frap_shortcuts_provider_new ("xfwm4");
  settings->priv->wm_channel = xfconf_channel_new ("xfwm4");
}



static void
xfwm_settings_constructed (GObject *object)
{
  XfwmSettings       *settings = XFWM_SETTINGS (object);
  const MenuTemplate *template;
  GtkTreeSelection   *selection;
  GtkCellRenderer    *renderer;
  GtkTargetEntry      target_entry[2];
  GtkListStore       *list_store;
  GtkTreeIter         iter;
  GtkWidget          *theme_name_treeview;
  GtkWidget          *title_font_button;
  GtkWidget          *title_align_combo;
  GtkWidget          *active_frame;
  GtkWidget          *active_box;
  GtkWidget          *hidden_frame;
  GtkWidget          *hidden_box;
  GtkWidget          *focus_delay_scale;
  GtkWidget          *click_to_focus_mode;
  GtkWidget          *raise_on_click_check;
  GtkWidget          *raise_on_focus_check;
  GtkWidget          *focus_new_check;
  GtkWidget          *box_move_check;
  GtkWidget          *box_resize_check;
  GtkWidget          *wrap_workspaces_check;
  GtkWidget          *wrap_windows_check;
  GtkWidget          *snap_to_border_check;
  GtkWidget          *snap_to_window_check;
  GtkWidget          *double_click_action_combo;
  GtkWidget          *snap_width_scale;
  GtkWidget          *wrap_resistance_scale;
  GtkWidget          *button;
  GValue              value = { 0, };
  GList              *children;
  GList              *list_iter;
  const gchar        *name;

  /* Style tab widgets */
  theme_name_treeview = glade_xml_get_widget (settings->priv->glade_xml, "theme_name_treeview");
  title_font_button = glade_xml_get_widget (settings->priv->glade_xml, "title_font_button");
  title_align_combo = glade_xml_get_widget (settings->priv->glade_xml, "title_align_combo");
  active_frame = glade_xml_get_widget (settings->priv->glade_xml, "active-frame");
  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");
  hidden_frame = glade_xml_get_widget (settings->priv->glade_xml, "hidden-frame");
  hidden_box = glade_xml_get_widget (settings->priv->glade_xml, "hidden-box");

  /* Style tab: theme name */
  {
    list_store = gtk_list_store_new (1, G_TYPE_STRING);
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (list_store), 0, 
                                     (GtkTreeIterCompareFunc) xfwm_settings_theme_sort_func, 
                                     NULL, NULL);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store), 0, GTK_SORT_ASCENDING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (theme_name_treeview), GTK_TREE_MODEL (list_store));
  
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (theme_name_treeview), 
                                                 0, _("Theme"), renderer, "text", 0, NULL);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (theme_name_treeview));
    g_signal_connect (selection, "changed", G_CALLBACK (xfwm_settings_theme_selection_changed), 
                      settings);
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    xfwm_settings_load_themes (settings);
  }

  /* Style tab: font */
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/title_font", G_TYPE_STRING, 
                          title_font_button, "font-name");

  /* Style tab: title alignment */
  {
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (title_align_combo));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (title_align_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (title_align_combo), renderer, "text", 0);

    list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_combo_box_set_model (GTK_COMBO_BOX (title_align_combo), GTK_TREE_MODEL (list_store));

    for (template = title_align_values; template->name != NULL; ++template)
      {
        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter, 0, template->name, 1, template->value, -1);
      }

    xfconf_channel_get_property (settings->priv->wm_channel, "/general/title_alignment", &value);
    xfwm_settings_title_alignment_property_changed (settings->priv->wm_channel, 
                                                    "/general/title_alignment", &value, settings);
    g_value_unset (&value);

    g_signal_connect (title_align_combo, "changed", 
                      G_CALLBACK (xfwm_settings_title_alignment_changed), settings);
    g_signal_connect (settings->priv->wm_channel, "property-changed::/general/title_alignment", 
                      G_CALLBACK (xfwm_settings_title_alignment_property_changed), settings);
  }

  /* Style tab: button layout */
  {
    target_entry[0].target = "_xfwm4_button_layout";
    target_entry[0].flags = GTK_TARGET_SAME_APP;
    target_entry[0].info = 2;

    target_entry[1].target = "_xfwm4_active_layout";
    target_entry[1].flags = GTK_TARGET_SAME_APP;
    target_entry[1].info = 3;

    gtk_drag_dest_set (active_frame, GTK_DEST_DEFAULT_ALL, target_entry, 2, GDK_ACTION_MOVE);

    g_signal_connect (active_frame, "drag-data-received", 
                      G_CALLBACK (xfwm_settings_active_frame_drag_data), settings);
    g_signal_connect (active_frame, "drag-motion", 
                      G_CALLBACK (xfwm_settings_active_frame_drag_motion), settings);
    g_signal_connect (active_frame, "drag-leave", 
                      G_CALLBACK (xfwm_settings_active_frame_drag_leave), settings);

    gtk_drag_dest_set (hidden_frame, GTK_DEST_DEFAULT_ALL, target_entry, 1, GDK_ACTION_MOVE);

    g_signal_connect (hidden_frame, "drag-data-received", 
                      G_CALLBACK (xfwm_settings_hidden_frame_drag_data), settings);

    children = gtk_container_get_children (GTK_CONTAINER (active_box));
    for (list_iter = children; list_iter != NULL; list_iter = g_list_next (list_iter))
      {
        button = GTK_WIDGET (list_iter->data);
        name = gtk_widget_get_name (button);

        if (name[strlen (name) - 1] == '|')
          {
            g_signal_connect (title_align_combo, "changed", 
                              G_CALLBACK (xfwm_settings_title_button_alignment_changed), button);
            xfwm_settings_title_button_alignment_changed (GTK_COMBO_BOX (title_align_combo), 
                                                          button);
          }

        g_object_set_data (G_OBJECT (button), "key_char", (gpointer) &name[strlen (name) - 1]);
        gtk_drag_source_set (button, GDK_BUTTON1_MASK, &target_entry[1], 1, GDK_ACTION_MOVE);

        g_signal_connect (button, "drag_data_get", 
                          G_CALLBACK (xfwm_settings_title_button_drag_data), 
                          target_entry[1].target);
        g_signal_connect (button, "drag_begin", G_CALLBACK (xfwm_settings_title_button_drag_begin),
                          NULL);
        g_signal_connect (button, "drag_end", G_CALLBACK (xfwm_settings_title_button_drag_end),
                          NULL);
        g_signal_connect (button, "button_press_event", 
                          G_CALLBACK (xfwm_settings_signal_blocker), NULL);
        g_signal_connect (button, "enter_notify_event", 
                          G_CALLBACK (xfwm_settings_signal_blocker), NULL);
        g_signal_connect (button, "focus",  G_CALLBACK (xfwm_settings_signal_blocker), NULL);
      }
  }
  g_list_free (children);

  children = gtk_container_get_children (GTK_CONTAINER (hidden_box));
  for (list_iter = children; list_iter != NULL; list_iter = g_list_next (list_iter))
    {
      button = GTK_WIDGET (list_iter->data);
      name = gtk_widget_get_name (button);

      g_object_set_data (G_OBJECT (button), "key_char", (gpointer) &name[strlen (name) - 1]);
      gtk_drag_source_set (button, GDK_BUTTON1_MASK, &target_entry[0], 1, GDK_ACTION_MOVE);

        g_signal_connect (button, "drag_data_get", 
                          G_CALLBACK (xfwm_settings_title_button_drag_data), 
                          target_entry[1].target);
        g_signal_connect (button, "drag_begin", G_CALLBACK (xfwm_settings_title_button_drag_begin),
                          NULL);
        g_signal_connect (button, "drag_end", G_CALLBACK (xfwm_settings_title_button_drag_end),
                          NULL);
        g_signal_connect (button, "button_press_event", 
                          G_CALLBACK (xfwm_settings_signal_blocker), NULL);
        g_signal_connect (button, "enter_notify_event", 
                          G_CALLBACK (xfwm_settings_signal_blocker), NULL);
        g_signal_connect (button, "focus",  G_CALLBACK (xfwm_settings_signal_blocker), NULL);
    }
  g_list_free (children);

  xfconf_channel_get_property (settings->priv->wm_channel, "/general/button_layout", &value);
  xfwm_settings_button_layout_property_changed (settings->priv->wm_channel,
                                                "/general/button_layout", &value, settings);
  g_value_unset (&value);

  /* Focus tab widgets */
  focus_delay_scale = glade_xml_get_widget (settings->priv->glade_xml, "focus_delay_scale");
  focus_new_check = glade_xml_get_widget (settings->priv->glade_xml, "focus_new_check");
  raise_on_focus_check = glade_xml_get_widget (settings->priv->glade_xml, "raise_on_focus_check");
  raise_on_click_check = glade_xml_get_widget (settings->priv->glade_xml, "raise_on_click_check");
  click_to_focus_mode = glade_xml_get_widget (settings->priv->glade_xml, "click_to_focus_mode");

  /* Focus tab */
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/focus_delay", G_TYPE_INT,
                          gtk_range_get_adjustment (GTK_RANGE (focus_delay_scale)), "value");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/click_to_focus", G_TYPE_BOOLEAN,
                          click_to_focus_mode, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/raise_on_click", G_TYPE_BOOLEAN,
                          raise_on_click_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/raise_on_focus", G_TYPE_BOOLEAN,
                          raise_on_focus_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/focus_new", G_TYPE_BOOLEAN, 
                          focus_new_check, "active");

  /* Advanced tab widgets */
  box_move_check = glade_xml_get_widget (settings->priv->glade_xml, "box_move_check");
  box_resize_check = glade_xml_get_widget (settings->priv->glade_xml, "box_resize_check");
  wrap_workspaces_check = glade_xml_get_widget (settings->priv->glade_xml, 
                                                "wrap_workspaces_check");
  wrap_windows_check = glade_xml_get_widget (settings->priv->glade_xml, "wrap_windows_check");
  snap_to_border_check = glade_xml_get_widget (settings->priv->glade_xml, "snap_to_border_check");
  snap_to_window_check = glade_xml_get_widget (settings->priv->glade_xml, "snap_to_window_check");
  double_click_action_combo = glade_xml_get_widget (settings->priv->glade_xml, 
                                                    "double_click_action_combo");
  snap_width_scale = glade_xml_get_widget (settings->priv->glade_xml, "snap_width_scale");
  wrap_resistance_scale = glade_xml_get_widget (settings->priv->glade_xml, 
                                                "wrap_resistance_scale");

  /* Advanced tab: double click action */
  {
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (double_click_action_combo));
    
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (double_click_action_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (double_click_action_combo), renderer, 
                                   "text", 0);

    list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_combo_box_set_model (GTK_COMBO_BOX (double_click_action_combo), 
                             GTK_TREE_MODEL (list_store));

    for (template = double_click_values; template->name != NULL; ++template)
      {
        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter, 0, template->name, 1, template->value, -1);
      }

    xfconf_channel_get_property (settings->priv->wm_channel, "/general/double_click_action", 
                                 &value);
    xfwm_settings_double_click_action_property_changed (settings->priv->wm_channel,
                                                        "/general/double_click_action",
                                                        &value, settings);
    g_value_unset (&value);

    g_signal_connect (double_click_action_combo, "changed", 
                      G_CALLBACK (xfwm_settings_double_click_action_changed),
                      settings);
    g_signal_connect (settings->priv->wm_channel, "property-changed::/general/double_click_action",
                      G_CALLBACK (xfwm_settings_double_click_action_property_changed), settings);
  }

  /* Advanced tab */
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/snap_width", G_TYPE_INT,
                          gtk_range_get_adjustment (GTK_RANGE (snap_width_scale)), "value");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/wrap_resistance", G_TYPE_INT, 
                          gtk_range_get_adjustment (GTK_RANGE (wrap_resistance_scale)), "value");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/box_move", G_TYPE_BOOLEAN, 
                          box_move_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/box_resize", G_TYPE_BOOLEAN, 
                          box_resize_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/wrap_workspaces", G_TYPE_BOOLEAN, 
                          wrap_workspaces_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/wrap_windows", G_TYPE_BOOLEAN, 
                          wrap_windows_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/snap_to_border", G_TYPE_BOOLEAN, 
                          snap_to_border_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/snap_to_windows", G_TYPE_BOOLEAN, 
                          snap_to_window_check, "active");
}



static void
xfwm_settings_finalize (GObject *object)
{
  XfwmSettings *settings = XFWM_SETTINGS (object);

  g_object_unref (settings->priv->wm_channel);
  g_object_unref (settings->priv->provider);
  g_object_unref (settings->priv->glade_xml);

  (*G_OBJECT_CLASS (xfwm_settings_parent_class)->finalize) (object);
}



static void
xfwm_settings_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  XfwmSettings *settings = XFWM_SETTINGS (object);

  switch (prop_id)
    {
    case PROP_GLADE_XML:
      g_value_set_object (value, settings->priv->glade_xml);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfwm_settings_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  XfwmSettings *settings = XFWM_SETTINGS (object);

  switch (prop_id)
    {
    case PROP_GLADE_XML:
      if (GLADE_IS_XML (settings->priv->glade_xml))
        g_object_unref (settings->priv->glade_xml);
      settings->priv->glade_xml = g_value_get_object (value);
      g_object_notify (object, "glade-xml");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



XfwmSettings *
xfwm_settings_new (void)
{
  XfwmSettings *settings = NULL;
  GladeXML     *glade_xml;

  glade_xml = glade_xml_new_from_buffer (xfwm4_dialog_glade, xfwm4_dialog_glade_length, NULL, NULL);

  if (G_LIKELY (glade_xml != NULL))
    settings = g_object_new (XFWM_TYPE_SETTINGS, "glade-xml", glade_xml, NULL);
#if !GLIB_CHECK_VERSION (2,14,0)
  xfwm_settings_constructed (G_OBJECT(settings));
#endif

  return settings;
}



static gint 
xfwm_settings_theme_sort_func (GtkTreeModel *model,
                               GtkTreeIter  *iter1,
                               GtkTreeIter  *iter2)
{
  gchar *str1 = NULL;
  gchar *str2 = NULL;

  gtk_tree_model_get (model, iter1, 0, &str1, -1);
  gtk_tree_model_get (model, iter2, 0, &str2, -1);

  if (str1 == NULL) str1 = g_strdup ("");
  if (str2 == NULL) str2 = g_strdup ("");

  if (g_str_equal (str1, DEFAULT_THEME))
    return -1;

  if (g_str_equal (str2, DEFAULT_THEME))
    return 1;

  return g_utf8_collate (str1, str2);
}



static void
xfwm_settings_load_themes (XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *view;
  GHashTable   *themes;
  GList        *keys;
  GList        *key;
  GDir         *dir;
  const gchar  *file;
  gchar       **theme_dirs;
  gchar        *filename;
  gchar        *active_theme_name;
  gint          i;

  themes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  view = glade_xml_get_widget (settings->priv->glade_xml, "theme_name_treeview");
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

  xfce_resource_push_path (XFCE_RESOURCE_THEMES, DATADIR G_DIR_SEPARATOR_S "themes");
  theme_dirs = xfce_resource_dirs (XFCE_RESOURCE_THEMES);
  xfce_resource_pop_path (XFCE_RESOURCE_THEMES);

  for (i = 0; theme_dirs[i] != NULL; ++i)
    {
      dir = g_dir_open (theme_dirs[i], 0, NULL);

      if (G_UNLIKELY (dir == NULL))
        continue;

      while ((file = g_dir_read_name (dir)) != NULL)
        {
          filename = g_build_filename (theme_dirs[i], file, "xfwm4", "themerc", NULL);

          if (g_file_test (filename, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) && 
              g_hash_table_lookup (themes, file) == NULL)
            {
              g_hash_table_insert (themes, g_strdup (file), GINT_TO_POINTER (1));
            }

          g_free (filename);
        }

      g_dir_close (dir);
    }

  active_theme_name = xfconf_channel_get_string (settings->priv->wm_channel, "/general/theme", DEFAULT_THEME);

  keys = NULL;
#if !GLIB_CHECK_VERSION (2,14,0)
  g_hash_table_foreach (themes, xfwm4_settings_get_list_keys_foreach, &keys);
#else
  keys = g_hash_table_get_keys (themes);
#endif

  for (key = keys; key != NULL; key = g_list_next (key))
    {
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, g_strdup (key->data), -1);

      if (G_UNLIKELY (g_str_equal (active_theme_name, key->data)))
        {
          gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (view)), 
                                          &iter);
        }
    }

  g_list_free (keys);
  g_free (active_theme_name);
  g_hash_table_unref (themes);
  g_strfreev (theme_dirs);
}



static GtkWidget *
xfwm_settings_create_dialog (XfwmSettings *settings)
{
  g_return_val_if_fail (XFWM_IS_SETTINGS (settings), NULL);
  return glade_xml_get_widget (settings->priv->glade_xml, "main-dialog");
}



static GtkWidget *
xfwm_settings_create_plug (XfwmSettings   *settings,
                           GdkNativeWindow socket_id)
{
  GtkWidget *plug;
  GtkWidget *child;

  g_return_val_if_fail (XFWM_IS_SETTINGS (settings), NULL);

  plug = gtk_plug_new (socket_id);
  gtk_widget_show (plug);

  child = glade_xml_get_widget (settings->priv->glade_xml, "plug-child");
  gtk_widget_reparent (child, plug);
  gtk_widget_show (child);

  return plug;
}



int 
main (int    argc, 
      char **argv)
{
  XfwmSettings *settings;
  GtkWidget    *dialog;
  GtkWidget    *plug;
  GError       *error = NULL;

  xfce_textdomain (GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

  if (G_UNLIKELY (!gtk_init_with_args (&argc, &argv, _("."), opt_entries, PACKAGE, &error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_print (_("%s: %s\nTry %s --help to see a full list of available command line options.\n"), 
                   PACKAGE, error->message, PACKAGE_NAME);
          g_error_free (error);
        }
      
      return EXIT_FAILURE;
    }

  if (G_UNLIKELY (opt_version))
    {
      g_print ("%s\n", PACKAGE_STRING);
      return EXIT_SUCCESS;
    }

  if (G_UNLIKELY (!xfconf_init (&error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_error (_("Failed to initialize xfconf. Reason: %s"), error->message);
          g_error_free (error);
        }

      return EXIT_FAILURE;
    }

  settings = xfwm_settings_new ();

  if (G_UNLIKELY (settings == NULL))
    {
      g_error (_("Could not create the settings dialog."));
      xfconf_shutdown ();
      return EXIT_FAILURE;
    }

  if (G_UNLIKELY (opt_socket_id == 0))
    {
      dialog = xfwm_settings_create_dialog (settings);
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
  else
    {
      plug = xfwm_settings_create_plug (settings, opt_socket_id);
      g_signal_connect (plug, "delete-event", G_CALLBACK (gtk_main_quit), NULL);

      gtk_main ();
    }

  g_object_unref (settings);

  xfconf_shutdown ();

  return EXIT_SUCCESS;
}



static void
xfwm_settings_theme_selection_changed (GtkTreeSelection *selection,
                                       XfwmSettings     *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GList        *rows;
  gchar        *theme;

  rows = gtk_tree_selection_get_selected_rows (selection, &model);

  if (G_UNLIKELY (g_list_length (rows) == 0))
    return;

  gtk_tree_model_get_iter (model, &iter, rows->data);
  gtk_tree_model_get (model, &iter, 0, &theme, -1);
  
  xfconf_channel_set_string (settings->priv->wm_channel, "/general/theme", theme);

  g_free (theme);
  g_list_foreach (rows, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (rows);
}



static void 
xfwm_settings_title_alignment_changed (GtkComboBox  *combo,
                                       XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *alignment;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  model = gtk_combo_box_get_model (combo);

  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (model, &iter, 1, &alignment, -1);

  xfconf_channel_set_string (settings->priv->wm_channel, "/general/title_alignment", alignment);

  g_free (alignment);
}



static void 
xfwm_settings_active_frame_drag_data (GtkWidget        *widget,
                                      GdkDragContext   *drag_context,
                                      gint              x,
                                      gint              y,
                                      GtkSelectionData *data,
                                      guint             info,
                                      guint             time,
                                      XfwmSettings     *settings)
{
  GtkWidget *source;
  GtkWidget *parent;
  GtkWidget *active_box;
  GList     *children;
  GList     *iter;
  gint       xoffset;
  gint       i;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  source = gtk_drag_get_source_widget (drag_context);
  parent = gtk_widget_get_parent (source);

  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");

  g_object_ref (source);
  gtk_container_remove (GTK_CONTAINER (parent), source);
  gtk_box_pack_start (GTK_BOX (active_box), source, info == 3, info == 3, 0);
  g_object_unref (source);

  xoffset = widget->allocation.x;

  children = gtk_container_get_children (GTK_CONTAINER (active_box));

  for (i = 0, iter = children; iter != NULL; ++i, iter = g_list_next (iter))
    if (GTK_WIDGET_VISIBLE (iter->data))
      if (x < (GTK_WIDGET (iter->data)->allocation.width / 2 + 
               GTK_WIDGET (iter->data)->allocation.x - xoffset))
        {
      	  break;
        }

  g_list_free (children);

  gtk_box_reorder_child (GTK_BOX (active_box), source, i);

  xfwm_settings_save_button_layout (settings, GTK_CONTAINER (active_box));
}



static gboolean 
xfwm_settings_active_frame_drag_motion (GtkWidget      *widget, 
                                        GdkDragContext *drag_context, 
                                        gint            x,
                                        gint            y, 
                                        guint           time,
                                        XfwmSettings   *settings)
{
  GtkWidget *active_box;
  GdkWindow *indicator;
  GList     *children;
  GList     *iter;
  gint       xoffset = widget->allocation.x;
  gint       height;
  gint       ix;
  gint       iy;

  g_return_val_if_fail (XFWM_IS_SETTINGS (settings), FALSE);

  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");
  children = gtk_container_get_children (GTK_CONTAINER (active_box));

  for (iter = children; iter != NULL; iter = g_list_next (iter))
    {
      if (GTK_WIDGET_VISIBLE (iter->data))
        {
          if (x < (GTK_WIDGET (iter->data)->allocation.width / 2 +
                   GTK_WIDGET (iter->data)->allocation.x - xoffset))
            {
              ix = GTK_WIDGET (iter->data)->allocation.x;
              break;
            }

          ix = GTK_WIDGET (iter->data)->allocation.x + GTK_WIDGET (iter->data)->allocation.width;
        }
    }

  g_list_free (children);

  ix -= INDICATOR_SIZE / 2 + 1;
  iy = active_box->allocation.y - INDICATOR_SIZE / 2 + 
       gtk_container_get_border_width (GTK_CONTAINER (active_box));

  indicator = g_object_get_data (G_OBJECT (active_box), "indicator_window");
  
  if (G_UNLIKELY (indicator == NULL))
    {
      height = active_box->allocation.height + INDICATOR_SIZE - 
               gtk_container_get_border_width (GTK_CONTAINER (active_box)) * 2;
      xfwm_settings_create_indicator (active_box, ix, iy, INDICATOR_SIZE, height);
    }
  else
    gdk_window_move (indicator, ix, iy);

  return FALSE;
}



static void
xfwm_settings_active_frame_drag_leave (GtkWidget      *widget,
                                       GdkDragContext *drag_context,
                                       guint           time,
                                       XfwmSettings   *settings)
{
  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  xfwm_settings_delete_indicator (glade_xml_get_widget (settings->priv->glade_xml, "active-box"));
}



static void
xfwm_settings_hidden_frame_drag_data (GtkWidget        *widget,
                                      GdkDragContext   *drag_context,
                                      gint              x,
                                      gint              y,
                                      GtkSelectionData *data,
                                      guint             info,
                                      guint             time,
                                      XfwmSettings     *settings)
{
  GtkWidget *source;
  GtkWidget *parent;
  GtkWidget *hidden_box;
  GtkWidget *active_box;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  
  source = gtk_drag_get_source_widget (drag_context);
  parent = gtk_widget_get_parent (source);
  
  hidden_box = glade_xml_get_widget (settings->priv->glade_xml, "hidden-box");
  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");

  if (G_UNLIKELY (parent == hidden_box))
    return;

  g_object_ref (source);
  gtk_container_remove (GTK_CONTAINER (parent), source);
  gtk_box_pack_start (GTK_BOX (hidden_box), source, FALSE, FALSE, 0);
  g_object_unref (source);

  xfwm_settings_save_button_layout (settings, GTK_CONTAINER (active_box));
}



static void
xfwm_settings_create_indicator (GtkWidget *widget,
                                gint       x,
                                gint       y,
                                gint       width,
                                gint       height)
{
  GdkWindowAttr attributes;
  GdkWindow    *indicator;
  GdkRegion    *shape;
  GdkPoint      points[9];
  gint          attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_COLORMAP | GDK_WA_VISUAL;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  attributes.title = NULL;
  attributes.event_mask = 0;
  attributes.x = x;
  attributes.y = y;
  attributes.width = width;
  attributes.height = height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.cursor = NULL;
  attributes.wmclass_name = NULL;
  attributes.wmclass_class = NULL;
  attributes.override_redirect = FALSE;

  indicator = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attr_mask);
  gdk_window_set_user_data (indicator, widget);
  g_object_set_data (G_OBJECT (widget), "indicator_window", indicator);

  points[0].x = 0;
  points[0].y = 0;
  points[1].x = width;
  points[1].y = 0;
  points[2].x = width / 2 + 1;
  points[2].y = width / 2;
  points[3].x = width / 2 + 1;
  points[3].y = height - 1 - width / 2;
  points[4].x = width;
  points[4].y = height;
  points[5].x = 0;
  points[5].y = height - 1;
  points[6].x = width / 2;
  points[6].y = height - 1 - width / 2;
  points[7].x = width / 2;
  points[7].y = width / 2;
  points[8].x = 0;
  points[8].y = 0;
  
  shape = gdk_region_polygon (points, 9, GDK_WINDING_RULE);
  gdk_window_shape_combine_region (indicator, shape, 0, 0);

  gdk_window_show (indicator);
  gdk_window_raise (indicator);
}



static void
xfwm_settings_delete_indicator (GtkWidget *widget)
{
  GdkWindow *indicator;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  indicator = g_object_get_data (G_OBJECT (widget), "indicator_window");

  if (G_LIKELY (indicator != NULL))
    {
      gdk_window_destroy (indicator);
      g_object_set_data (G_OBJECT (widget), "indicator_window", NULL);
    }
}



static void
xfwm_settings_title_button_drag_data (GtkWidget        *widget,
                                      GdkDragContext   *drag_context,
                                      GtkSelectionData *data,
                                      guint             info,
                                      guint             time,
                                      const gchar      *atom_name,
                                      XfwmSettings     *settings)
{
  gtk_widget_hide (widget);
  gtk_selection_data_set (data, gdk_atom_intern (atom_name, FALSE), 8, NULL, 0);
}



static void
xfwm_settings_title_button_drag_begin (GtkWidget      *widget,
                                       GdkDragContext *drag_context)
{
  GdkPixbuf *pixbuf;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  pixbuf = xfwm_settings_create_icon_from_widget (widget);
  gtk_drag_source_set_icon_pixbuf (widget, pixbuf);
  g_object_unref (pixbuf);

  gtk_widget_hide (widget);
}



static void
xfwm_settings_title_button_drag_end (GtkWidget      *widget,
                                     GdkDragContext *drag_context)
{
  gtk_widget_show (widget);
}



static gboolean
xfwm_settings_signal_blocker (GtkWidget *widget)
{
  return TRUE;
}



static GdkPixbuf *
xfwm_settings_create_icon_from_widget (GtkWidget *widget)
{
  GdkWindow *drawable;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  drawable = GDK_DRAWABLE (gtk_widget_get_parent_window (widget));
  return gdk_pixbuf_get_from_drawable (NULL, drawable, NULL, 
                                       widget->allocation.x, widget->allocation.y, 0, 0,
                                       widget->allocation.width, widget->allocation.height);
}



static void
xfwm_settings_button_layout_property_changed (XfconfChannel *channel,
                                              const gchar   *property,
                                              const GValue  *value,
                                              XfwmSettings  *settings)
{
  GtkWidget   *active_box;
  GtkWidget   *hidden_box;
  GtkWidget   *button;
  GList       *children;
  GList       *iter;
  const gchar *str_value;
  const gchar *key_char;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  hidden_box = glade_xml_get_widget (settings->priv->glade_xml, "hidden-box");
  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");
  
  gtk_widget_set_app_paintable (active_box, FALSE);
  gtk_widget_set_app_paintable (hidden_box, FALSE);

  children = gtk_container_get_children (GTK_CONTAINER (active_box));

  /* Move all buttons to the hidden list, except for the title */
  for (iter = children; iter != NULL; iter = g_list_next (iter))
    {
      button = GTK_WIDGET (iter->data);
      key_char = (const gchar *) g_object_get_data (G_OBJECT (button), "key_char");

      if (G_LIKELY (key_char[0] != '|'))
        {
          g_object_ref (button);
          gtk_container_remove (GTK_CONTAINER (active_box), button);
          gtk_box_pack_start (GTK_BOX (hidden_box), button, FALSE, FALSE, 0);
          g_object_unref (button);
        }
    }

  g_list_free (children);

  children = g_list_concat (gtk_container_get_children (GTK_CONTAINER (active_box)),
                            gtk_container_get_children (GTK_CONTAINER (hidden_box)));

  /* Move buttons to the active list */
  for (str_value = g_value_get_string (value); *str_value != '\0'; ++str_value)
    for (iter = children; iter != NULL; iter = g_list_next (iter))
      {
        button = GTK_WIDGET (iter->data);
        key_char = (const gchar *) g_object_get_data (G_OBJECT (button), "key_char");

        if (g_str_has_prefix (str_value, key_char))
          {
            g_object_ref (button);
            gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (button)), button);
            gtk_box_pack_start (GTK_BOX (active_box), button, 
                                key_char[0] == '|', key_char[0] == '|', 0);
            g_object_unref (button);
          }
      }

  g_list_free (children);

  gtk_widget_set_app_paintable (active_box, TRUE);
  gtk_widget_set_app_paintable (hidden_box, TRUE);
}



static void 
xfwm_settings_title_alignment_property_changed (XfconfChannel *channel,
                                                const gchar   *property,
                                                const GValue  *value,
                                                XfwmSettings  *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *combo;
  gchar        *alignment;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  combo = glade_xml_get_widget (settings->priv->glade_xml, "title_align_combo");
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  if (gtk_tree_model_get_iter_first (model, &iter))
    {
      do 
        {
          gtk_tree_model_get (model, &iter, 1, &alignment, -1);
          
          if (G_UNLIKELY (g_str_equal (alignment, g_value_get_string (value))))
            {
              g_free (alignment);
              gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
              break;
            }

          g_free (alignment);
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }
}



static void 
xfwm_settings_save_button_layout (XfwmSettings *settings,
                                  GtkContainer *container)
{
  GList        *children;
  GList        *iter;
  const gchar **key_chars;
  gchar        *value;
  gint          i;
  
  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  children = gtk_container_get_children (container);
  
  key_chars = g_new0 (const char *, g_list_length (children) + 1);

  for (i = 0, iter = children; iter != NULL; ++i, iter = g_list_next (iter))
    key_chars[i] = (const gchar *) g_object_get_data (G_OBJECT (iter->data), "key_char");

  value = g_strjoinv ("", (gchar **) key_chars);

  xfconf_channel_set_string (settings->priv->wm_channel, "/general/button_layout", value);

  g_list_free (children);
  g_free (key_chars);
  g_free (value);
}



static void 
xfwm_settings_double_click_action_changed (GtkComboBox  *combo,
                                           XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *value;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  model = gtk_combo_box_get_model (combo);
  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (model, &iter, 1, &value, -1);

  xfconf_channel_set_string (settings->priv->wm_channel, "/general/double_click_action", value);

  g_free (value);
}



static void
xfwm_settings_title_button_alignment_changed (GtkComboBox *combo,
                                              GtkWidget   *button)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *value;
  float         align = 0.5f;

  model = gtk_combo_box_get_model (combo);
  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (model, &iter, 1, &value, -1);

  if (g_str_equal (value, "left"))
    align = 0.0f;
  else if (g_str_equal (value, "right"))
    align = 1.0f;

  gtk_button_set_alignment (GTK_BUTTON (button), align, 0.5f);

  g_free (value);
}



static void 
xfwm_settings_double_click_action_property_changed (XfconfChannel *channel,
                                                    const gchar   *property,
                                                    const GValue  *value,
                                                    XfwmSettings  *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *combo;
  gchar        *current_value;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  combo = glade_xml_get_widget (settings->priv->glade_xml, "double_click_action_combo");
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  if (G_LIKELY (gtk_tree_model_get_iter_first (model, &iter)))
    {
      do 
        {
          gtk_tree_model_get (model, &iter, 1, &current_value, -1);

          if (G_UNLIKELY (g_str_equal (current_value, g_value_get_string (value))))
            {
              g_free (current_value);
              gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
              break;
            }

          g_free (current_value);
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }
}