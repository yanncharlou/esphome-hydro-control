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
``` 
3. Eventuellement récupérer le dashboard entier se trouvant dans le dossier hydro-control/home-assistant/dashboard.yaml