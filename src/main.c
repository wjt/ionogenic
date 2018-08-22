#include <string.h>
#include <flatpak.h>

static void
mark (const char *where)
{
  g_autoptr(GDateTime) now = g_date_time_new_now_local ();
  g_autofree char *now_str = g_date_time_format (now, "%F %T%:z");
  g_printerr ("%s %s\n", now_str, where);
}
#define MARK() mark (G_STRFUNC)

static gboolean
transaction_operation_error_cb (FlatpakTransaction          *transaction,
                                FlatpakTransactionOperation *operation,
                                const GError                *error,
                                int                          details,
                                gpointer                     user_data)
{
  g_warning ("%s", error->message);
  return FALSE;
}

static gboolean
install_bundle (FlatpakInstallation *installation,
                GFile               *bundle,
                GCancellable        *cancellable,
                GError             **error)
{
  MARK();

  g_autoptr(FlatpakTransaction) transaction =
    flatpak_transaction_new_for_installation (installation, cancellable, error);
  if (!transaction)
    return FALSE;

  if (!flatpak_transaction_add_install_bundle (transaction, bundle, NULL, error))
    return FALSE;

  /* We don't care about the runtime, this is just being used as a
   * representative workload for background automatic updates.
   */
  flatpak_transaction_set_no_pull (transaction, TRUE);
  /* This has no effect for bundles: reinstallation still fails. */
  flatpak_transaction_set_reinstall (transaction, TRUE);

  g_signal_connect (transaction, "operation-error",
                    G_CALLBACK (transaction_operation_error_cb), NULL);

  return flatpak_transaction_run (transaction, cancellable, error);
}

static gboolean
uninstall_app (FlatpakInstallation *installation,
               const char          *app_id,
               GCancellable        *cancellable,
               GError             **error)
{
  MARK ();
  const char *arch = "x86_64";
  const char *branch = "stable";

  return flatpak_installation_uninstall (installation,
                                         FLATPAK_REF_KIND_APP,
                                         app_id,
                                         arch,
                                         branch,
                                         NULL,
                                         NULL,
                                         cancellable,
                                         error);
}

int
main (int    argc,
      char **argv)
{
  g_autoptr(GFile) bundle = NULL;
  g_autofree char *app_id = NULL;
  g_autoptr(FlatpakInstallation) system = NULL;
  g_autoptr(GCancellable) cancellable = NULL;
  g_autoptr(GError) error = NULL;

  if (argc != 2)
    g_error ("Usage: %s org.example.AppID.bundle", argv[0]);

  bundle = g_file_new_for_commandline_arg (argv[1]);
  app_id = g_file_get_basename (bundle);
  *strrchr (app_id, '.') = '\0';

  if (!(system = flatpak_installation_new_system (NULL, &error)))
    g_error ("failed to open system installation: %s", error->message);

  if (!uninstall_app (system, app_id, cancellable, &error))
    {
      g_message ("initial uninstall failed: %s", error->message);
      g_clear_error (&error);
    }

  while (install_bundle (system, bundle, cancellable, &error) &&
         uninstall_app (system, app_id, cancellable, &error))
    {
      /* pass */
    }

  if (error != NULL)
    {
      g_warning ("%s", error->message);
      return 1;
    }

  return 0;
}
