I tried using [multitime](http://tratt.net/laurie/src/multitime/) to launch LibreOffice 15 times (flushing disk caches between each run). I wrote [this little program](https://github.com/wjt/ionogenic/blob/master/src/main.c) to install/uninstall a flatpak bundle in a loop, to see if this makes a difference versus running the `flatpak install`/`flatpak uninstall` commands from the shell (it does).

Yoga 900 measurements are of running `multitime -r 'echo 3 | sudo tee /proc/sys/vm/drop_caches' -n 15 flatpak run  org.libreoffice.LibreOffice --terminate_after_init` which means:

* 15 trials
* Flush disk caches after every run
* Runs LO as unprivileged desktop session user to avoid potential root weirdness

Mission measurements used `-s 5` to sleep up to 5 seconds between runs, and `-n 30` for .. maybe better data and squashing of outliers? Or should we actually be worried about the Max?

## Bfq

### Unloaded

                Mean        Std.Dev.    Min         Median      Max
    real        2.819       0.018       2.780       2.816       2.850
    user        0.017       0.005       0.007       0.017       0.026
    sys         0.016       0.005       0.008       0.016       0.026

### Loaded with `sudo ./_build/src/iogenic ~/Downloads/org.gnome.Lollypop.flatpak`:

                Mean        Std.Dev.    Min         Median      Max
    real        3.410       0.782       2.869       2.962       5.447
    user        0.017       0.007       0.001       0.017       0.028
    sys         0.017       0.006       0.008       0.016       0.031

### Loaded with `sudo sh -c 'while true; do flatpak install -y /home/wjt/Downloads/org.gnome.Lollypop.flatpak; flatpak uninstall -y org.gnome.Lollypop/x86_64/stable; done'`:

                Mean        Std.Dev.    Min         Median      Max
    real        4.425       3.026       2.867       3.011       14.486
    user        0.017       0.004       0.008       0.018       0.023
    sys         0.017       0.004       0.010       0.017       0.025

### Loaded with `sudo ionice -c3 ./_build/src/iogenic ~/Downloads/org.gnome.Lollypop.flatpak`:

                Mean        Std.Dev.    Min         Median      Max
    real        3.063       0.244       2.873       2.946       3.596
    user        0.017       0.007       0.001       0.017       0.027
    sys         0.018       0.008       0.005       0.016       0.032

### Loaded with `sudo ionice -c3 sh -c 'while true; do flatpak install -y /home/wjt/Downloads/org.gnome.Lollypop.flatpak; flatpak uninstall -y org.gnome.Lollypop/x86_64/stable; done'`

                Mean        Std.Dev.    Min         Median      Max
    real        3.155       0.247       2.932       3.026       3.730
    user        0.016       0.006       0.006       0.019       0.024
    sys         0.019       0.007       0.008       0.020       0.034

## Cfq

### Unloaded:

                Mean        Std.Dev.    Min         Median      Max
    real        2.735       0.018       2.704       2.735       2.774
    user        0.015       0.005       0.006       0.014       0.023
    sys         0.019       0.005       0.010       0.019       0.027

### Loaded with `sudo ./_build/src/iogenic ~/Downloads/org.gnome.Lollypop.flatpak`:

                Mean        Std.Dev.    Min         Median      Max
    real        3.093       0.311       2.807       3.017       3.952
    user        0.016       0.006       0.006       0.016       0.030
    sys         0.022       0.006       0.006       0.022       0.030

### Loaded with `sudo sh -c 'while true; do flatpak install -y /home/wjt/Downloads/org.gnome.Lollypop.flatpak; flatpak uninstall -y org.gnome.Lollypop/x86_64/stable; done'`:

                Mean        Std.Dev.    Min         Median      Max
    real        3.047       0.199       2.828       2.974       3.622
    user        0.017       0.004       0.009       0.017       0.024
    sys         0.021       0.005       0.012       0.020       0.032

### Loaded with `sudo ionice -c3 ./_build/src/iogenic ~/Downloads/org.gnome.Lollypop.flatpak`:

                Mean        Std.Dev.    Min         Median      Max
    real        2.968       0.084       2.827       2.966       3.158
    user        0.017       0.004       0.010       0.018       0.026
    sys         0.021       0.005       0.013       0.021       0.029

## To check:

* BFQ, CFQ with fio-generated loads, optionally ioniced
