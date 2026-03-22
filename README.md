# Hard Time – PS Vita Port

Fan-made port di **Hard Time** (© Mat Dickie 2007) da Blitz3D a C++ / PS Vita.

> **Nota legale:** il port usa gli asset originali del gioco (texture, mesh, suoni).
> Devi possedere una copia regolare di Hard Time per usarli.

---

## Build con GitHub Actions

Push su `main` → workflow automatico:
1. Container `vitasdk/vitasdk:latest`
2. Installa `vitaGL glm zlib` via vdpm
3. Compila con CMake/Ninja
4. Pubblica `HardTime.vpk` come artefatto scaricabile

Per creare una release taggata:
```bash
git tag v0.1.0 && git push origin v0.1.0
```

---

## Build locale

```bash
export VITASDK=/usr/local/vitasdk
vdpm vitaGL glm zlib

mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake
make -j4
# → build/HardTime.vpk
```

---

## Installazione su PS Vita

1. Scarica `HardTime.vpk` da Actions → Artifacts
2. Trasferiscilo con VitaShell (USB o FTP)
3. Installa: seleziona VPK → Install

---

## Cartella dati – cosa mettere in `ux0:data/HardTime/`

**Devi copiare gli asset del gioco originale** dalla cartella di installazione
di Hard Time su PC (tipicamente `C:\Program Files\Hard Time\`) mantenendo
esattamente la struttura seguente:

```
ux0:data/HardTime/
│
├── Characters/
│   ├── Models/
│   │   ├── Model01.3ds   (20 modelli personaggio)
│   │   ├── Model02.3ds
│   │   └── ... Model20.3ds
│   └── Sequences/
│       ├── Standard01.3ds   (animazioni base – OBBLIGATORIO)
│       ├── Standard02.3ds
│       ├── Standard03.3ds
│       ├── Standard04.3ds
│       ├── Move_Execute.3ds  (mosse grapple esecutore)
│       └── Move_Receive.3ds  (mosse grapple ricevente)
│
├── World/
│   ├── Block/
│   │   └── Block.3ds         (blocco celle prigione)
│   ├── Yard/
│   │   ├── Yard.3ds           (cortile)
│   │   └── Outro.3ds          (scena finale uscita)
│   ├── Study/
│   │   └── Study.3ds
│   ├── Hospital/
│   │   └── Hospital.3ds
│   ├── Kitchen/
│   │   └── Kitchen.3ds
│   ├── Hall/
│   │   └── Hall.3ds
│   ├── Workshop/
│   │   └── Workshop.3ds
│   ├── Toilet/
│   │   └── Toilet.3ds
│   ├── Courtroom/
│   │   └── Courtroom.3ds      (scena tribunale)
│   └── Sprites/
│       ├── Particle.bmp        (sprite particelle FX)
│       ├── Pool.png            (sprite pozze sangue/acqua)
│       └── Shadow.png          (ombra personaggi)
│
├── Weapons/
│   ├── Plank.3ds
│   ├── MetalBar.3ds
│   ├── Knife.3ds
│   ├── Cleaver.3ds
│   ├── Sword.3ds
│   ├── Axe.3ds
│   ├── Sledge.3ds
│   ├── Shield.3ds
│   ├── Pistol.3ds
│   ├── Shotgun.3ds
│   ├── Rifle.3ds
│   ├── Bottle.3ds
│   ├── Rock.3ds
│   ├── Barbell.3ds
│   ├── Chain.3ds
│   ├── BaseballBat.3ds
│   ├── TNT.3ds
│   ├── Extinguisher.3ds
│   ├── Crowbar.3ds
│   ├── Spanner.3ds
│   ├── Tray.3ds
│   ├── Book.3ds
│   ├── Broom.3ds
│   └── Mop.3ds
│
├── Textures/
│   └── (tutte le .png/.bmp del gioco originale)
│
├── Sound/
│   ├── Theme.wav
│   ├── Browse.wav
│   ├── Select.wav
│   ├── Confirm.wav
│   ├── Cancel.wav
│   ├── Void.wav
│   ├── Trash.wav
│   ├── Camera.wav
│   ├── Computer.wav
│   ├── Cash.wav
│   ├── Paper.wav
│   ├── Murmur.wav
│   ├── Cheer.wav
│   ├── Jeer.wav
│   ├── Ambience/
│   │   ├── Quiet.wav
│   │   ├── Yard.wav
│   │   ├── Hall.wav
│   │   └── Crowd.wav
│   ├── Movement/
│   │   ├── Fall.wav
│   │   ├── Swing.wav
│   │   ├── Step01.wav … Step06.wav
│   │   ├── Pain01.wav … Pain08.wav
│   │   ├── Shuffle01.wav … Shuffle03.wav
│   │   ├── Impact01.wav … Impact06.wav
│   │   └── Agony01.wav … Agony05.wav
│   ├── Weapons/
│   │   ├── Shot01.wav … Shot05.wav
│   │   └── Ricochet01.wav … Ricochet05.wav
│   ├── Props/
│   │   ├── Explosion.wav
│   │   ├── Splash.wav
│   │   └── Blaze.wav
│   └── World/
│       ├── Door01.wav … Door03.wav
│       ├── Buzzer.wav
│       ├── Bell.wav
│       ├── Ring.wav
│       ├── Alarm.wav
│       ├── Tanoy.wav
│       └── Basket.wav
│
└── Data/    ← creata automaticamente dal gioco per i salvataggi
```

> Trovi tutti questi file nella cartella di installazione di Hard Time su PC.
> Copia l'intera struttura così com'è.

---

## Mappatura controlli

| Funzione | Vita |
|---|---|
| Muoversi | Stick sinistro |
| Attaccare | ✕ Cross |
| Difendere / Grapple | ○ Circle |
| Lanciare arma | □ Square |
| Raccogliere / Usare | △ Triangle |
| Pausa | START |
| Menu | SELECT (tenuto) |

---

## Sistemi portati

| Sistema | File | Stato |
|---|---|---|
| Vita init + vitaGL | main.cpp | ✅ |
| Variabili globali (300+) | values.cpp | ✅ |
| Save / Load | data.cpp | ✅ |
| Input controller | ai.cpp | ✅ |
| AI (5 agende) | ai.cpp | ✅ |
| Animazioni (200+ clip, grapple, gestures) | anims.cpp | ✅ |
| Fisica (camminata, corsa, attacchi, stagger) | moves.cpp | ✅ |
| Particelle 14 tipi, pool, esplosioni | particles.cpp | ✅ |
| Caricamento personaggi, armi, mondo | players.cpp | ✅ |
| Sistema crimini + warrant + tribunale | crimes.cpp | ✅ |
| Dialoghi interattivi (300 scenari chiave) | promos.cpp | ✅ |
| Missioni (13 tipi) | missions.cpp | ✅ |
| Loop gameplay + HUD completo + scena finale | gameplay.cpp | ✅ |
| Tutti i menu | menus.cpp | ✅ |
| Scene graph 3D vitaGL | render3d.cpp | ✅ |

### Ancora da fare per v1.0

| Cosa | Note |
|---|---|
| Loader mesh .3DS | Integrare assimp |
| Loader texture PNG/BMP | Integrare stb_image |
| Audio WAV reale | Integrare dr_wav + SceAudio |
| Collisioni 3D | Sweep sphere/AABB |
| Animazioni bone | Collegare assimp skeleton a ExtractAnimSeq |
| Font in-game | FreeType o sprite font sheet |
| Editor personaggi | Portare Editor.bb |
| Location rimanenti | Kitchen, Hall, Workshop, Toilet, City |
