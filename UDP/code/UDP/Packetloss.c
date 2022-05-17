void Packetloss(PakkettenOntvangen, AantalBytes){

double PacketLoss_1, PacketLoss_2, PacketLoss_Totaal;

PacketLoss_1 = PakketenOntvangen / 100;
PacketLoss_2 = 1 - PacketLoss_1;
PacketLoss_Totaal = PacketLoss_2 * 100;
printf("Er is %0.8f% packetloss.\n", PacketLoss_Totaal);

}
