///////////////////////////////////////////////////////////////////////
//
// MessageQueue
//
// Synchronized queue for messages
//
// Mike Dixon
//
// Dec 2002
//
////////////////////////////////////////////////////////////////////////

import java.util.*;

public class MessageQueue

{
    
    private LinkedList _list;
    
    public MessageQueue() {
	_list = new LinkedList();
    }

    public void push(Object o) {
	synchronized(_list) {
	    _list.addFirst(o);
	}
    }

    public Object pop() {
	synchronized(_list) {
	    if (_list.size() > 0) {
		return _list.removeLast();
	    } else {
		return null;
	    }
	}
    }

}
