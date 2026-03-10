int main() {
    json::object jdoc;
    jdoc.set("title", "Test");
    jdoc.set("margins", "10 10 10 10");

    json::array objs;
    json::object obj;
    obj.set("type", "bool");
    obj.set("title", "Test");
    obj.set("name", "optTest");
    objs.append(obj);

    obj.set("type", "string");
    obj.set("title", "Input");
    obj.set("name", "leString");
    objs.append(obj);

    obj.set("type", "boolopt");
    obj.set("title", "Testopt");
    obj.set("name", "optTestGroup");

    json::array subarr;
    json::object subobj;
    subobj.set("name", "optA");
    subobj.set("text", "A");
    subarr.append(subobj);
    subobj.set("name", "optB");
    subobj.set("text", "B");
    subarr.append(subobj);
    obj.set("data", subarr);
        
    objs.append(obj);

    json::object entryobj;
    entryobj.set("type", "group");
    entryobj.set("title", "Test");
    entryobj.set("data", objs);

    json::array entries;
    entries.append(entryobj);
    jdoc.set("data", entries);

    optbox box(jdoc);

    printf("{:h}\n", box.exec());

    return 0;
}
