### Opdracht:
Je maakt een 4 applicaties (UDP Client, UDP Server, TCP Client en TCP Server) die via Berkley sockets in C met elkaar over internet kunnen communiceren. De 2 Server applicaties en de 2 Client applicaties mogen op je laptop draaien.

#### UDP Smartphone sensor stream analyzer (15%) [App]

##### Welke onderdelen zijn behaald van de UDP Client ?
 
 - [X] Werkende Client applicatie die aantal te sturen packetten via UDP stuurt (bewijs Packet Sender) [2/15]
 
##### Welke onderdelen zijn behaald van de UDP Server ?
 
 - [X] Werkende Server applicatie die gegevens via UDP ontvangt (bewijs Packet Sender) [2/15]
 
 - [X] CSV van ontvangen stream wordt aangemaakt [1/15]
 
 - [X] Het aantal pakketten dat ontvangen is wordt bijgehouden [1/15]
 
 - [X] Er is een aantal te ontvangen pakketten in te stellen [1/15]
 
 - [X] Het tijdsverschil tussen eerste en laatste pakket wordt getoond [1/15]
 
 - [X] *Er is een time-out in te stellen [2/15]
 
 - [X] *Bij time-out het aantal effectief ontvangen t.o.v. verwacht aantal wordt getoond met berekening van packetloss [2/15]
 
 - [ ] *De gegevens worden op de server geparsed en een min, max en avg worden bijgehouden [2/15]
 
 - [ ] *Statistische gegevens worden in een bestand bewaard [1/15]
 
#### TCP Chatbox met geschiedenis via HTTP (25%)

##### Welke onderdelen zijn behaald van de TCP Client ?
 
 - [*] Client kan TCP connectie maken (bewijs via WireShark) [1/25]
 
 - [*] Client kan TCP packetten succesvol sturen en ontvangen (bewijs via WireShark) [1/25]
 
 - [X] Client stuurt de door de gebruiker ingegeven berichten door (bewijs Packet Sender) [1/25]
 
 - [ ] *Er kan tegelijk berichten gestuurd en ontvangen worden op de Client zonder afgesproken beurtrol (e.g. multi-threaded) [4/25]
 
 ##### Welke onderdelen zijn behaald van de TCP Server ?
 
 - [X] Server kan luisteren naar een poort voor inkomende TCP connecties [1/25]
 
 - [X]  Server accepteert verbinding en praat met verbonden client over TCP [1/25]
 
 - [X]  Server applicatie stuurt ontvangen bericht door naar alle andere deelnemers [1/25]
 
 - [X]  Server stuurt niet naar de afzender zijn eigen bericht [1/25]
 
 - [ ]  Server stuurt de afzender informatie mee met het bericht (i.e. IP-adres + poort van zender) [2/25]
 
 - [ ]  Server stuurt de laatste 16 chatberichten naar de client bij nieuwe verbinding [2/25]
 
 - [ ]  Server kan HTTP communicatie uitvoeren met de webserver (bewijs via WireShark) [2/25]
 
 - [ ] *Server stuurt over HTTP de chatberichten door naar de webserver (bewijs via WireShark) [2/25]
 
 - [ ]  *Server vraagt de laatste 16 chatberichten via HTTP aan de webserver (bij start) [2/25]
 
 - [X] *Er kunnen meerdere Clients simultaan verbonden zijn en chatten met elkaar (i.e. assynchroon of synchroon verwerkt op de server) [4/25]
 
