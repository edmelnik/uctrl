def sendData(values):
    output = ""
    try:
        for value in values:
            output += value
            output += " "
        print(output)
        return 1
    except IndexError:
        return -1
    
