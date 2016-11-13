package org.hpms.mw.channel;

public interface IMessageHandler {

   public void decode( short msgId, CoDec decoder );
}
