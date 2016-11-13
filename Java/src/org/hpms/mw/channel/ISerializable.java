package org.hpms.mw.channel;

public interface ISerializable<T extends ISerializable<T>> {

   IFactory<T> getFactory();
}
