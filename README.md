# esphome-hydro-control
Contrôleur d'installation hydroponique sous ESP Home / Home Assistant

## Installation

Je n'ai pas pris le temps de créer un véritable package vu que c'est un bricolage perso en constante évolution.

1. Télécharger le contenu du repo dans le dossier /config/esphome/hydro-control
2. Créer une nouvelle entité ESPHome avec le code suivant :
```yaml
esphome:
  name: hydro-control-1
  friendly_name: Contrôle Hydroponique 1
  compile_process_limit: 1

packages:
  hydro-control: !include hydro-control/main.yaml

wifi:
#  ssid: !secret wifi_ssid_jardin
#  password: !secret wifi_password_jardin
  networks:
  - ssid: !secret wifi_ssid_jardin
    password: !secret wifi_password_jardin
  - ssid: !secret wifi_ssid
    password: !secret wifi_password

substitutions:
  
  # PH Calibration (no way to make it from device by now)
  ph_7: "1.814"
  ph_4: "0.586"

  #EC calibration is set directly in sensor parameters
  ec_ratio: "1.828"

  # Pump calibration
  # 60s = 93.5ml -> ratio = 60000 / 93.5 = 645.2
  pump1_ml_to_ms_ratio: "645.2"
  # 60s = 103.8ml -> ratio = 60000 / 103.8 = 578.03
  pump2_ml_to_ms_ratio: "578.03"
  # 60s = 98.4 -> ratio = 60000 / 98.4 == 609.76
  pump3_ml_to_ms_ratio: "609.76"
``` 
3. Eventuellement récupérer le dashboard entier se trouvant dans le dossier hydro-control/home-assistant/dashboard.yaml