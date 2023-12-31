# ircbots
A comprehensive collection of classic IRC bots, mostly from the VladBot family.
Many of these bots were written in a time where cpu and memory was quite 
expensive and having a scripting engine or inefficient code would be very
detrimental. Version control systems, public repositories and pre-packaged bots
delivered as part of an operating system were rare.

People downloaded source code, forked it to suit their needs and there are most
likely many mods, rewrites and other changes that are lost to time.

In some cases forks were done and each fork incorporated all wanted changes 
from the original code bases. Putting new changes in a separate file made it
easier to update and merge source code. This is most likely whye the incubot
has a vlad-ons, crc-ons, mega-ons, tbt-ons and crc-ons source code files. The
code is the original bot and the work of four other forks / coders.

Many of the bots would rename the files and often remove documentation and 
credits. This could be to give an impression of having developed all the code
themselves, but it could also have been a way to avoid the original developers
receiving bug reports and help requests for code they did not write or have
access to.

I started using Incubots in 2000-2001 and have been using a fork of this bot
since then. In 2023 I started digging into the history of the bot and went
looking for forks of the bot. I found a lot more than I had expected. It was
a big surprise to me that the well-known Energy Mech bot was from the VladBot
family.

## VladBot family tree
After some research, this is the known family tree for the VladBot family of irc
bots.
```
Bot 0.01                             Early version of Hopbot, same author.
PCBot 0.01                           Intermediate between bot and hopbot.
Hopbot                               Very basic bot, less than 15kb.
+--VladBot 1.x                       Only got the source for 1.9
   VladBot 2.1.c
   +--VladBot 2.1.c-ariela           A few very minor modifications.
   VladBot 2.1.g
   +--ChRoniCBot                     Stricter on channel security.
   |  +--IncuBot 1.02c               The branch incubus worked on.
   |  |  +--Incubot 1.02c-Byxnet     Compiler fixes, minor bug fixes.
   |  |  +--Mikribot                 My own branch.
   |  |  +--Revbot                   Revzoot's branch for #quake.   
   |  IncuBot 1.03b-beta             Has more code from The Black
   VladBot 2.1.h
   +--VladBot-pHb-Nefertari          French translation.
   +--HipBot                         Stripped away some features.
   +--VladBot-2.1.h-m                Removed note support.
   +--Achille
   |  +--VladBot 2.1-sk3             The Patrocle bot
   VladBot-2.10
   +--fRitE 3.0                      Forked by Offspring.
   |  fRitE 4.1.x
   |  +--fRitE+                      Continuation by Pyber.
   VladBot 2.x                      
   +--Johbot-10.2                    Binary + config/docs only, 3 versions.
   +--HackBot 0.9-alpha 13           Coded by ComStud.
   |  +--TnT2 5.7                    Bin+docs. Has code from Johbot.
   |  Hackbot x.x                    Coded by ComStud.
   |  +--ComBot x.x                  Coded by ComStud.
   |  |  +--ComBot-Deluxe            Only mentioned online on macademia.
   |  |  +--DweeB0t                  Fork by Piraeus.
   |  |  +--StelBot 1.0              StealthBot.
   |  |  |  +--RockBot               Fixes alleged backdoors in Stel.
   |  |  |  |  +--StarBot            Starlink network service bot.
   |  |  |  StelBot 1.2              Polished and cleaned up.
   |  |  ComBot 2.05                 Base for energybot.
   |  |  +--EnergyBot                Ancestor of emech.
   |  |     +--EnergyMech 2.0.3      Code by Polygon.
   |  |         +--EnergyMech SG     Starglider Class fork by Proton.
   |  |            +--Minimech       Low memory fork by same author.
   |  |            +--Extramech-gudelia Fork with services support and a game.
   |  +--HofBot 1.03f                Fork by KuLaT nmashudi@brazos.pe.utexas.edu   
   |  |  HofBot 2.0b                 
   |  +--DivBot 2.0a                 Binary, docs
   |     DivBot 4.0                  Binary, docs
   VladBot 3.x branch                No known forks off of this one.
```

## Credits
Many of the forks removed the credits to earlier authors, giving an incomplete
view of who did what. All of the bots should credit 'Coyote Peyote' and all but
the two earliest should credit Ivo and the rest of the VladBot team.

### Bot, PCBot, HopBot
Coyote Peyote

### VladBot
Ivo 'VladDrac' van der Wijk, Richard 'Richie\_B' van der Berg, President, mozgy 
\+ the HopBot author.
AIX port: Dathes

### Ariela
H\_I (parmenti@loria.fr) + VladBot authors.

### VladBot-pHb-Nefertari
Most likely Philippe 'PhBb' Brand + Vladbot authors. PhB is also mentioned in
the credits for the Dancer irc bot.


### ChroniC Bot
Robert 'BadCRC' Alto, The-Black, Adrian 'MeGALiTH' Cable, Brian 'Wendigo' Hess 
\+ the VladBot authors.

### IncuBot
Incubus, Eggdrop team + the ChroniC bot authors.

### Revbot
Revzoot + the Incubot authors.

### Mikribot
Kim Roar 'Zokum' Foldøy Hauge, Reep, Colin 'cph' Phipps + the Incubot authors.

### Hackbot & Combot
Christopher 'CoMSTuD' Behrens + the VladBot authors.

### TnT2 
ttone + Hackbot authors and johbot authors.

### Energy Mech 2.x
Energy Mech Team, Mong (RoverBot code), Pup (LamerBawt code) + the Combot authors

### Johbot
joh!chihhung@wam.umd.edu (John Kao), + VladBot authors, possibly others.

### Divbot
curl@rabi.phys.columbia.edu + Hackbot authors.
