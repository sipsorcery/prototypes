using System;
using System.Collections.Generic;
using System.Text;

namespace KurentoEchoClient
{
    public enum KurentoMethodsEnum
    {
        create,
        invoke,
        ping,
        release,
        subscribe,
        unsubscribe,
    }

    public class KurentoResult
    {
        public string value { get; set; }
        public string sessionId { get; set; }
    }

    public class KurentoEvent
    {
        public KurentoEventData data { get; set; }
        public string @object { get; set;}
        public string type { get; set; }
    }

    public class KurentoEventData
    {
        public string source { get; set; }
        public string[] tags { get; set; }
        public string timestamp { get; set; }
        public string timestampMillis { get; set; }
        public string type { get; set; }
    }
}
