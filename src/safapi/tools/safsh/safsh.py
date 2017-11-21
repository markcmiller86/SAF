#!/usr/bin/env python
##
## Copyright(C) 1999-2005 The Regents of the University of California.
##     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
##     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
##     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
##     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
##     dissemination, commercialization  through formal licensing, or other  disposition under terms of
##     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
##     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
##     University.
## 
## Copyright(C) 1999-2005 Sandia Corporation.  
##     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
##     on behalf of the U.S. Government.  Export  of this program may require a license from the United
##     States Government.
## 
## Disclaimer:
##     This document was  prepared as an account of  work sponsored by an agency  of  the United States
##     Government. Neither the United States  Government nor the United States Department of Energy nor
##     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any
##     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the
##     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process
##     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference
##     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark,
##     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement,
##     recommendation, or favoring by the  United States Government   or the University of  California.
##     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
##     the  United  States Government or  the   University of California   and shall  not be  used  for
##     advertising or product endorsement purposes.
## 
## 
## Active Developers:
##     Peter K. Espen              SNL
##     Eric A. Illescas            SNL
##     Jake S. Jones               SNL
##     Robb P. Matzke              LLNL
##     Greg Sjaardema              SNL
## 
## Inactive Developers:
##     William J. Arrighi          LLNL
##     Ray T. Hitt                 SNL
##     Mark C. Miller              LLNL
##     Matthew O'Brien             LLNL
##     James F. Reus               LLNL
##     Larry A. Schoof             SNL
## 
## Acknowledgements:
##     Marty L. Barnaby            SNL - Red parallel perf. study/tuning
##     David M. Butler             LPS - Data model design/implementation Spec.
##     Albert K. Cheng             NCSA - Parallel HDF5 support
##     Nancy Collins               IBM - Alpha/Beta user
##     Linnea M. Cook              LLNL - Management advocate
##     Michael J. Folk             NCSA - Management advocate 
##     Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
##     Wilbur R. Johnson           SNL - Early developer
##     Quincey Koziol              NCSA - Serial HDF5 Support 
##     Celeste M. Matarazzo        LLNL - Management advocate
##     Tyce T. McLarty             LLNL - parallel perf. study/tuning
##     Tom H. Robey                SNL - Early developer
##     Reinhard W. Stotzer         SNL - Early developer
##     Judy Sturtevant             SNL - Red parallel perf. study/tuning 
##     Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
## 



import os, exceptions, string, sys, StringIO, cmd, UserDict, UserList, UserString, re, ss
import pdb
import Tkinter
import Tree





def plain(text):
    """Remove boldface formatting from text."""
    return text
    # return re.sub('.\b', '', text)

class Mystdout(StringIO.StringIO):

    
    
    def __init__(self, buf = ''):
        StringIO.StringIO.__init__(self,buf)
        self.pos = 0
        # self.num_lines = os.environ.get('LINES',25)
        self.num_lines = 15
        if isinstance(self.num_lines,type("")):
            self.num_lines = string.atoi(self.num_lines)
        self.num_lines = self.num_lines - 1

        self.buf = ""
        
    def write(self,s):
        # _stdout.write(s)
        self.buf += s
                
        # self.ttypager(s)

    def _flush(self):
        global _safview_mode
        global _safview_write
        if self.buf != "":
            if (_safview_mode):
                _safview_write.ttypager(self.buf)
            else:
                self.ttypager(self.buf)
        self.buf = ""
        
    def _reset_pos(self):
        self.pos = 0
        
    def ttypager(self,text):
        global _stdout
        global _stdin
        global _stderr
        global _stdin_fileno

        """Page through text on a text terminal."""
        lines = string.split(plain(text), '\n')
        num_text_lines = len(lines) - 1

        try:
            import tty
            fd = _stdin_fileno
            old = tty.tcgetattr(fd)
            tty.setcbreak(fd)
            getchar = lambda: _stdin.read(1)
        except (ImportError, AttributeError):
            tty = None
            getchar = lambda: _stdin.readline()[:-1][:1]

        try:
            r = os.environ.get('LINES',25)
            if isinstance(r,type("")):
                r = string.atoi(r)
            r = inc = r - 1
            
            # _stdout.write(string.join(lines[:inc], '\n') + '\n')
            _stdout.write(string.join(lines[:inc], '\n'))
            self.pos += num_text_lines
            if lines[r:]:
                _stdout.write('\n')

            
            while lines[r:]:
                rf = r * 1.0
                perc = rf/num_text_lines * 100
                _stdout.write('[7m-- more -- (%2.0f%%) [m' % perc)
                try:
                    _stdout.flush()
                except:
                    pass
                c = getchar()
                goback = 1

                if c in ['q', 'Q']:
                    _stdout.write('\r                     \r')
                    break
                elif c in ['\r', '\n']:
                    _stdout.write('\r                     \r' + lines[r] + '\n')
                    r = r + 1
                    continue
                if c in ['b', 'B', '\x1b']:
                    r = r - inc - inc
                    if r < 0:
                        r = 0
                
                _stdout.write('\r                     \r')
                _stdout.write(string.join(lines[r:r+inc], '\n') + '\n')
                r = r + inc
                if not lines[r:]:
                    break

        finally:
            if tty:
                tty.tcsetattr(fd, tty.TCSAFLUSH, old)



class ExpertPrompt(UserString.UserString):
    """
    Generate a prompt
        The string generated here replaces the interactive prompt (ps1). The UserString class
        stores the portion of the prompt in the self.data attribute.
    """

    def __init__(self,refstring):
        UserString.UserString.__init__(self,refstring+'# ')
        self.prompt_orig = refstring
        self.prompt_num = 0

    def __repr__(self):
        return '@' + repr(self.data)
    

    def __str__(self):
        # eai global _pagerout
        self.prompt_num += 1
        # eai sys.stdout = _pagerout
        # eai try:
        # eai    _pagerout._flush()
        # eai  except:
        # eai     pass
        return "\n%d_" % self.prompt_num + self.data 



def test_pager():
    """ Only used to test ttypager; currently not used """
    text = ""
    for i in range(0,200):
        text = text + "%dalsd\tjasalsdjasd alsjdasd ladjasldkjasd laksjdasldj alsdjasldkjas d\n" % i
    text = text + "1234"
    print text
        

class SAFList(UserList.UserList, object):

    def __init__(self,reflist):
        if isinstance(reflist,SAFList):
            UserList.UserList.__init__(self,reflist.data)
        else:
            UserList.UserList.__init__(self,reflist)
            
        self.type = 'List'
        for i in range(0,len(self.data)):
            if isinstance(self.data[i],SAFObjectDict):
                item_name = self.data[i].name
                self.__dict__[item_name] = self.data[i]
            else:
                try:
                    item_name = self.data[i].name
                except AttributeError:
                    pass
                else:
                    self.__dict__[item_name] = self.data[i]

    def __call__(self):
        # self.sets()
        # self.list()
        self.names()

    def append(self,item):
        UserList.UserList.append(self,item)
        if isinstance(item,SAFObjectDict):
            self.__dict__[item.name] = item
        
    def old__call__(self):
        i = 0
        for j in range(0,len(self.data)):
            item = self.data[j]         
            if isinstance(item,SAFObjectDict):
                print "item %d:" % i
                item()
            if isinstance(item,SAFList):
                print "item %d:" % i
                item()
            i += 1 ; print
            # print i
            
    def list(self):
        for i in self.data:
            if isinstance(i,SAFObjectDict):
                if i.type == "SAF_Set" or i.type == 'SAF_Cat' or i.type == 'SAF_Field':
                    print "\t\t",i.name
            if isinstance(i,SAFList):
                # i.sets()
                i.list()


    def names(self):
        num = 0
        for i in self.__dict__.keys():
            if i != 'data' and i != 'type':
                print "\t\t",num,':',i
                num += 1

    def describe(self):
        # self.__call__()
        # self.sets()
        # self.list()
        self.names()

    def __repr__(self):
        # self.sets()
        # return repr(self.data)
        self()
        return ""
        
    def __str__(self):
        # self.sets()
        # self.list()
        self.names()
        return ""

    def _private_(self):
        for i in self.data:
            if not isinstance(i,SAFObjectDict) and not isinstance(i,SAFList):
                print i
                
#    def __len__(self):
#        nums = 0
#        for i in self.data:
#            if isinstance(i,SAFObjectDict) or isinstance(i,SAFList):
#                nums += 1
#        return nums


    def __eq__(self,instance):
        return 0

class CollectionList(SAFList):
    def __init__(self,refdict):
        SAFList.__init__(self,refdict)

    def names(self):
        num = 0
        for i in range(0,len(self)):
            print "\t\t",num,':\t',self[i].name
            num += 1

    def ok(self):
        pass

class SetList(CollectionList):
    def __init__(self,reflist):
        SAFList.__init__(self,reflist)
        

class FieldList(CollectionList):
    def __init__(self,refdict):
        SAFList.__init__(self,refdict)
        # super(FieldList, self).ok()

    def read(self):
        return self

class StateList(FieldList):
    def __init__(self,refdict):
        SAFList.__init__(self,refdict)

    def read(self):
        return self

    def names(self):
        num = 0
        for i in range(0,len(self)):
            spacer = ' ' * (10 - len(self[i].name))
            print "\t\t",num,':\t',self[i].name,spacer,self.timesteps[num]
            num += 1




class SubsetList(SAFList):
    
    def __init__(self,refdict):
        SAFList.__init__(self,refdict)
        self.type = 'SubsetList'
        ## self._coll_dict = coll_dict


    def old_list(self):
        if self.data != []:
            print "\tsubsets:"
            SAFList.list(self)

    def list(self):
        if self.data != []:
            for i in self.data:
                if isinstance(i,SAFObjectDict):
                    if i.type == "SAF_Set":
                        print "\t\t",i.name
                if isinstance(i,SAFList):
                    # i.sets()
                    i.list()

    def names(self):
        print
        tmp_dict = {}
        for i in range(0,len(self)):
            subset_name = self[i].name
            if not tmp_dict.has_key(subset_name):
                tmp_dict[subset_name] = []
            tmp_dict[subset_name].append(self[i].subset_on.name)
        for i in range(0,len(self)):
            num = 0
            subset_name = self[i].name
            if tmp_dict.has_key(subset_name):
                coll_string = (15-len(subset_name)) * " "
                coll_string += "on "
                num_colls = len(tmp_dict[subset_name]);
                for j in range(0,num_colls):
                    if num == 0:
                        if tmp_dict[subset_name][j] != None:
                            coll_string += tmp_dict[subset_name][j]
                            num += 1
                    else:
                        coll_string += ", " + tmp_dict[subset_name][j]
                print "\t",subset_name, coll_string
                del tmp_dict[subset_name]
                
    def work_names(self):
      print

      _coll_dict = None
      ## if self.__dict__['_coll_dict'] == None:
      if _coll_dict == None:
          for i in range(0,len(self)):
              print "\t",self[i].name
      else:
          _coll_dict = self.__dict__['_coll_dict']
          for i in range(0,len(self)):
              num = 0
              subset_name = self[i].name
              coll_string = (15-len(subset_name)) * " "
              coll_string += "on "
              for j in range(0,len(_coll_dict[subset_name])):
                  if num == 0:
                      coll_string += _coll_dict[subset_name][j]
                      num += 1
                  else:
                      coll_string += ", " + _coll_dict[subset_name][j]
              print "\t",subset_name, coll_string

                
            
class SubList(SubsetList):
    def __init__(self,reflist):
        if isinstance(reflist,SAFList):
            UserList.UserList.__init__(self,reflist.data)
        else:
            UserList.UserList.__init__(self,reflist)
            
        self.type = 'SubList'
        for i in range(0,len(self.data)):
            if isinstance(self.data[i],SAFObjectDict):
                item_name = self.data[i].name
                self.__dict__[item_name] = self.data[i]

class DictWrap(UserDict.UserDict):
    """ 
    Class used create a custom Dictionary 
    """

#    def __init__(self,refdict):
#        UserDict.UserDict.__init__(self,refdict)
                                                                                                                    
    def __init__(self,refdict):
        if isinstance(refdict,SAFObjectDict):
            self.data = refdict.data
        else:
            self.data = refdict
                                                                                                                    
                                                                                                                    
    def __getattr__(self,name):
        # invoked when <classintance>.<name> is  specified on value retrieval
        # example
        #       a = x.s  # Ivokes __getattr__(x,"s")
        return self.data[name]


class SAFObjectDict(DictWrap):
     
    def __init__(self,initdict):
        if isinstance(initdict,SAFObjectDict) or isinstance(initdict,type({})):
            DictWrap.__init__(self,initdict)
        else:
            print "SAFObjectDict type error"
            del self

    def __getattr__(self,name):
        # invoked when <classintance>.<name> is  specified on value retrieval
        # example
        #       a = x.s  # Ivokes __getattr__(x,"s")
        if self.data != {}:
            return self.data[name]


    
    def describe(self):
        for i in self.data.keys():
            print "\t",i

            
    def old_describe(self):
        for i in self.data.keys():
            if i != '_db' and i[0] != '_':
                try:
                    if self.data['type'] == 'SAF_Set':
                        print "\t",i
                    elif i == 'set':
                        print "\t",i,":",self.data[i].name
                    elif i == 'subsets':
                        if self.data[i] != None:
                            print "\tsubsets:"
                            self.data[i].list()
                    else:
                            print "\t",i,":",self.data[i]
                except TypeError:
                    print "SAFObjectDict describe(): TypeError"
                    return
                except KeyError:
                    # "Is this a subset dictionary?"
                    print "\t",i
                    
        return

    def _items(self):
        for i in self.data.keys():
            item = self.data[i]
            if item.type == 'SAF_Set' or item.type == 'SAF_Cat' or item.type == 'SAF_Field':
                print "\t\t",item.name
                
    def __str__(self):
        # print "SAFObjectDict __str__"
        # self.describe()
        # return repr(self.data)
        self()
        return ""

    def __repr__(self):
        # print "SAFObjectDict __repr__",self.data.keys()
        # self.describe()
        # return repr(self.data)
        self()
        return ""

    def __setattr__(self,name,value):
        # invoked when <classintance>.<name> is  specified on assignment
        # example
        #       x.s=a  # Ivokes __setattr__(x,"s",a)

        try:
            l = _db._is_locked()
            l = 0
        except (NameError, AttributeError):
            pass
        else:
            if l == 1:
                return
        
        if name == "data":
            self.__dict__[name] = value
        else:
            if name in self.__dict__.keys():
                print "attribute already exists"
            else:
                self.__dict__['data'][name] = value
                
            
    def __call__(self):
        # print "SAFObjectDict __call__"
        self.describe()

    def __eq__(self,instance):
        return 0


class Collection(SAFObjectDict):
    def __init__(self,refdict):
        SAFObjectDict.__init__(self,refdict)
    def read(self):
                                                     
        # base_space = self.base_space
                                                                                                                   
        # try:
            # mydb = base_space._db
        # except:
            # pass
        # else:
            # ss.switch_db( mydb._py_handle )
        
        the_data = ss.get_collection_data(self.data)

#        if type(the_data) is type([]):
#            if len(the_data) == 1:
#                the_data = Field(the_data[0])
#            else:
#                for i in range(0,len(the_data)):
#                    the_data[i] = Field(the_data[i])
#                the_data = FieldList(the_data)
#                                                                                                                   
#            for i in range(0,len(base_space.fields)):
#                if base_space.fields[i]._handle == self._handle:
#                    the_data.name = base_space.fields[i].name
#                    base_space.fields[i] = the_data
#                                                                                                                   
#            the_data.base_space = base_space
#            the_data = ss.get_field_data(the_data.data)
                                                                                                                   
        return the_data

    def describe(self):
        for i in ['name','set','type','count','subsets']:
            try:
                if self.data['type'] == 'SAF_Set':
                    print "\t",i
                elif i == 'set':
                    print "\t",i," :",self.data[i].name
                elif i == 'count':
                    print "\t",i," :",self.data[i]
                elif i == 'subsets':
                    if self.data[i] != None:
                        print "\tsubsets:"
                        self.data[i].list()
                else:
                        print "\t",i,":",self.data[i]
            except TypeError:
                print "SAFObjectDict describe(): TypeError"
            except KeyError:
                pass

class SAFdbs(SAFObjectDict):

    def __init__(self,refdict):
        self.__dict__['locks'] = {}
        SAFObjectDict.__init__(self,refdict)

    def describe(self):

            print '\t',self.data[key],':',key

    def _lock(self,name):
        self.__dict__['locks'][name] = 1

    def _unlock(self,name):
        self.__dict__['locks'][name] = 0



class DataBase(SAFObjectDict):

    def __init__(self,refdict,varname=""):
        self.__dict__['_py_handle'] = refdict
        print "creating python database instance..."
        SAFObjectDict.__init__(self,refdict)

        self.__dict__['_allsets'] = {}
        self.__dict__['_allsets_list'] = SetList([])
        
        for i in range(0,len(refdict['_topsets'])):
            if (_debug):print "in DataBase _topsets:",i, refdict['_topsets'][i]
            refdict['_topsets'][i] = Set(refdict['_topsets'][i])
            if (_debug):print "in DataBase post _topsets",i, refdict['_topsets'][i]
            refdict['_topsets'][i]._db = self
                        
        self.__dict__['_topsets'] = SAFList(refdict['_topsets'])
        
        try:
            for i in range(0,len(refdict['_suites'])):
                if (_debug):print "in DataBase _suites",i, refdict['_suites'][i]
                refdict['_suites'][i] = Suite(refdict['_suites'][i])
                if (_debug):print "in DataBase post _suites",i, refdict['_suites'][i]
                refdict['_suites'][i]._db = self
                        
            self.__dict__['_suites'] = SAFList(refdict['_suites'])

        except TypeError:
            pass

        for set in self._topsets:
            setname = set['name']
            self._topsets.__dict__[setname] = set
            
            self._get_allsets(set)
            self.__dict__['_allsets'] = SAFObjectDict(self.__dict__['_allsets'])
            
        keys = self.__dict__['_allsets'].keys()
        keys.sort()
        for key in keys:
            self.__dict__['_allsets'][key]._db = self
            self.__dict__['_allsets_list'].append(self.__dict__['_allsets'][key])
            
        self.topsets = self._topsets
        self.sets = self._allsets_list
        self.suites = self._suites
        self.num_all_sets = len(self.sets)

    def _lock(self):
        _working_dbs._lock(_working_dbs[self.data['name']])

    def _unlock(self):
        _working_dbs._unlock(_working_dbs[self.data['name']])

    def _is_locked(self):
        if _working_dbs.__dict__['locks'][_working_dbs[self.data['name']]] == 1:
            return 1
        else:
            return 0
        
        
    def _set_globals(self):
        # global _db1
        global _db
        global _top
        global _tops
        global _num_top_sets
        global _topsets
        global topsets
        global allsets
        global _allsets
        global _working_dbs
        global safcmd
        global suites

        # _db1 = self
        _db = self
        _topsets = _db._topsets
        _tops = _topsets
        _top = _tops[0]
        _num_topsets = len(_tops)
        _allsets = self._allsets_list
        topsets = _db._topsets
        allsets = _allsets
        suites  = self.suites

        try:
            sys.ps1.data = sys.ps1.prompt_orig + '_' + _working_dbs[self.data['name']] + '# '
        except KeyError:
            for key in globals().keys():
                if isinstance(globals()[key],DataBase):
                    if globals()[key].data['_db_handle'] == self.data['_db_handle']:
                        if key[0] != '_':
                            _working_dbs[self.data['name']] = key
            sys.ps1.data = sys.ps1.prompt_orig + '_' + _working_dbs[self.data['name']] + '# '
            
        safcmd.prompt = safcmd.prompt_orig + '_' + _working_dbs[self.data['name']] + '> '
        
        self._lock()

    def ss_topsets(self,arg = None):
        print "DataBase.ss_topsets\n"
        if( arg != None):
            print arg
        the_arg = self.data
        return ss.topsets(the_arg)

    def _get_allsets(self,top):
        self._allsets[top.name] = top
        for subset in top.subsets:
            self._get_allsets(subset)

    def my_topsets(self):
        if isinstance(self._topsets,SAFList):
            if len(self._topsets) <= 2:
                print "\ttopsets",":\t",self._topsets[0].name
            else:
                print "\ttopsets:"
                for i in range(0,len(self._topsets)):
                    print "\t\t\t",self._topsets[i].name
                    #self._topsets[i].describe()
                    
    def my_suites(self):
        if isinstance(self._suites,SAFList):
            if len(self._suites) <= 2:
                print "\tsuites",":\t",self._suites[0].name
            else:
                print "\tsuites:"
                for i in range(0,len(self._suites)):
                    print "\t\t\t",self._suites[i].name

    def __str__(self):
        self.describe()
        return ""
        
    def describe(self):
        # print "\t",'name:\t',self.data['name']
        # print "\t",'type:\t',self.data['type']
        # for key in self.data.keys():
        for key in ['name', 'type', 'num_top_sets', 'num_all_sets', 'num_suites', 'suites', 'topsets']:
            if key == 'topsets':
                self.my_topsets()
            elif key == 'suites':
                self.my_suites()
            elif key == 'sets':
                continue
            elif key[0] != '_':
                if len(key) > 8:
                    print "\t",key,":\t",self.data[key]
                else:
                    print "\t",key,":\t\t",self.data[key]
        # print "\ttop sets:"
        # for i in range(0,len(self['_topsets'])):
        #    print "\t\t",self._topsets[i]['name']
        # self.my_topsets()
        # print "\t",'num_topsets:\t',self.data['num_top_sets']

        self._set_globals()

    def SierraStats( self ):

        try:
            if self.topsets[0].coordinates is not None:
                try:
                    print "Coordinates per node        =\t%d" % (self.topsets[0].coordinates.num_comps)
                except:
                    pass
                try:
                    print "Number of nodes             =\t%d" % ( (self.topsets[0].coordinates.data_size / self.topsets[0].coordinates.num_comps) )
                except:
                    pass
                try:
                    print "Number of elements          =\t%d"    % ( self.topsets[0].elements.count )
                except:
                    pass
                try:
                    print "Number of element blocks    =\t%d"      % ( self.topsets[0].element_blocks.count )
                    print
                except:
                    pass

                try:
                    print "Number of nodal point sets  =\t%d"   %  ( len(self.topsets[0].nodes.subsets) )
                except:
                    pass
                try:
                    print "Number of element side sets =\t%d"   %  ( len(self.topsets[0].boundary_condition_sets.subsets) )
                except:
                    pass

                print
                
        except:
            pass


class Field(SAFObjectDict):

    def __init__(self,refdict):
        SAFObjectDict.__init__(self,refdict)
        if self.num_comps > 0:
            
            for i in range(0,len(self._comp_fields)):
                self._comp_fields[i] = Field(self._comp_fields[i])
                comp_name = self._comp_fields[i].name
                self.__dict__[comp_name] = self._comp_fields[i]
            self._comp_fields = FieldList(self._comp_fields)
            self.components = self._comp_fields

            # self.resolve_base_space()
            
    def read_indirect(self):
        base_space = self.base_space
        the_data = ss.get_field_data(self.data)
        
        if type(the_data) is type([]):
            if len(the_data) == 1:
                the_data = Field(the_data[0])
            else:
                for i in range(0,len(the_data)):
                    the_data[i] = Field(the_data[i])
                the_data = FieldList(the_data)

            for i in range(0,len(base_space.fields)):
                if base_space.fields[i]._handle == self._handle:
                    the_data.name = base_space.fields[i].name
                    base_space.fields[i] = the_data

        the_data.base_space = base_space            

        return the_data

    def read(self):
        base_space = self.base_space

        try:
            mydb = base_space._db
        except:
            pass
        else:
            ss.switch_db( mydb._py_handle )
        
        the_data = ss.get_field_data(self.data)
        
        if type(the_data) is type([]):
            if len(the_data) == 1:
                the_data = Field(the_data[0])
            else:
                for i in range(0,len(the_data)):
                    the_data[i] = Field(the_data[i])
                the_data = FieldList(the_data)

            for i in range(0,len(base_space.fields)):
                if base_space.fields[i]._handle == self._handle:
                    the_data.name = base_space.fields[i].name
                    base_space.fields[i] = the_data

            the_data.base_space = base_space
            the_data = ss.get_field_data(the_data.data)

        return the_data
    
    def describe(self):
        # for i in self.data.keys():
        for i in ['name','coeff_assoc','data_size','data_type','interleave', 'num_comps','type','components']:
            if i[0] != '_':

                if i == 'components':
                    if self.num_comps > 0:
                        print '\tcomponents :'
                        self.data[i].list()
                else:
                    if( len(i) < 8 ):
                        print "\t",i,"\t\t:",self.data[i]
                    else:
                        print "\t",i,"\t:",self.data[i]
                        
        return

    def set_base_space(self, set):
        if type(self.base_space) is type(""):
            self.base_space = set

    def resolve_base_space(self):

        if type(self.base_space) is type(""):
            for i in allsets:
                if i.name == self.base_space:
                    self.base_space = i
                    break

class State(Field):

    def read(self):
        base_space = self.base_space
        ss.switch_db( base_space._db._py_handle )
        the_state_data = ss.get_field_data(self.data)
        the_data = the_state_data['_states']
        the_timesteps = the_state_data['_timesteps']

        
        if type(the_data) is type([]):
            
            for i in range(0,len(the_data)):
                for j in range(0,len(the_data[i])):
                    the_data[i][j] = Field(the_data[i][j])
                    the_data[i][j].resolve_base_space()
                the_data[i] = FieldList(the_data[i])
                the_data[i].name = "State %d" % (i)
                the_data[i].timestep = the_timesteps[i]
                
            the_data = StateList(the_data)
            the_data.base_space = base_space
            the_data.timesteps = the_timesteps
            for i in range(0,len(base_space.states)):
                try:
                    if base_space.states[i]._handle == self._handle:
                        the_data.name = self.name
                        base_space.states[i] = the_data
                except:
                    pass
        else:
            return None
       
        return the_data    
    
class Set(SAFObjectDict):

    def __init__(self,refdict):
            
        if refdict == {}:
            self.data = {}
            return

        SAFObjectDict.__init__(self,refdict)
        
        # self.collections = CollectionList(self.collections)
        # self.fields = FieldList(self.fields)


##        self._subset_ptrs = []
##        self['subset_of'] = Set({})
##        self['subset_on'] = Collection({})
       
        # convert to self.fields to FieldList 
        if( not isinstance(self.fields,FieldList) ):
            if self.fields != None:
                for i in range(0,len(self.fields)):

                    if type(self.fields[i]) == type({}):

                        if (self.fields[i]['coeff_assoc'] == "stategroups"):
                            self.fields[i] = State(self.fields[i])
                        else:
                            self.fields[i] = Field(self.fields[i])

                        self.fields[i].set_base_space(self)

                        field_name = self.fields[i].name
                        self.__dict__[field_name] = self.fields[i]
                        # self.fields[i].base_space = self

                        try:
                            components = self.fields[i].components
                        except:
                            pass
                        else:
                            for comp in components:
                                comp.set_base_space(self)
                            
                    
                self.fields = FieldList(self.fields)
            else:
                self.fields = FieldList([])

            
        # convert self.collections to CollectionList 
        if( not isinstance(self.collections,CollectionList)  ):

            self._subset_ptrs = []
            self['subset_of'] = Set({})
            self['subset_on'] = Collection({})

            try:
                for i in range(0,len(self.collections)):
                    self.collections[i] = Collection(self.collections[i])
                    coll_name = self.collections[i].name
                    self.__dict__[coll_name] = self.collections[i]
                    if self.collections[i].subsets != None:
                        for j in range(0,len(self.collections[i].subsets)):
                            raw = Set(self.collections[i].subsets[j])
                            self.collections[i].subsets[j] = Subset(self.collections[i].subsets[j])
                            ### self.collections[i].subsets[j]['subset_of'] = self
                            self.collections[i].subsets[j]['subset_of'] = self

                            for coll_num in range(0,len(self.collections[i].subsets[j].collections)):
                                if self.collections[i].subsets[j].collections[coll_num].name ==  self.collections[i].name:
                                    self.collections[i].subsets[j]['subset_on'] = self.collections[i].subsets[j].collections[coll_num]

                            self._subset_ptrs.append(raw)

                            self.collections[i].subsets = SubsetList( self.collections[i].subsets )


                    # self.collections[i] = SAFObjectDict(self.collections[i])
                    self.collections[i].set = self

                self.collections = CollectionList(self.collections)
            except:
                self.collections = None
        
            # self._subset_ptrs = SAFObjectDict(self._subset_ptrs)

        self._subset_ptrs = SubList(self._subset_ptrs )
        self.subsets = self._subset_ptrs

                


    def describe(self):
       
			
        if self.data == {}:
            return
        
        keys = self.__dict__.keys()

        # for key in ['name','type','subset_of','subset_on','collections','subsets','fields']:
        for key in ['name','collections','subsets','fields']:
        # for key in ['name','type','collections','subsets']:
        
            if key == 'collections':
                self.my_collections()
            elif key == 'name':
                print "\tname:\t\t",self.data[key]
            elif key == 'fields':
                self.my_fields()
            elif key == 'subsets':
                # self.my_subsets()
                if len(self.subsets) >= 1:
                    self.list_names('subsets',self.subsets)
            elif key == 'subset_of':
                if self.subset_of.data != {}:
                    print "\tsubset_of:\t",self.subset_of.name
            elif key == 'subset_on':
                if self.subset_on.data != {}:
                    print "\tsubset_on:\t",self.subset_on.name

            elif key[0] != '_':
                if len(key) > 8:
                    print "\t",key,":\t",self.data[key]
                else:
                    print "\t",key,":\t\t",self.data[key]


        
    def my_collections(self,arg = None):
        if self.collections.data == []:
            return
        
        if isinstance(self.collections,SAFList):
            if len(self.collections) < 2:
                print "\tcollections:\t",self.collections[0].name
            else:
                print "\tcollections:"
                for i in range(0,len(self.collections)):
                    print "\t\t\t",self.collections[i].name

    def my_fields(self,arg = None):
        if self.fields.data == []:
            return
        
        if isinstance(self.fields,SAFList):
            if len(self.fields) < 2:
                print "\tfields:\t\t",self.fields[0].name
            else:
                print "\tfields:"
                for i in range(0,len(self.fields)):
                    print "\t\t\t",self.fields[i].name

    def list_names(self,list_name,list):
        if isinstance(list,SAFList):
            if len(list) < 2:
                print "\t",list_name,":\t",list[0].name
            else:
                print "\t",list_name,":"
                for key in list.__dict__.keys():
                    if key != 'data' and key != 'type':
                        print "\t\t\t",key
                        
    def read(self):
        
        if self.subset_on.data == {}:
            print "no kids"
            return

        coll = self.subset_on
        parent = self.subset_of
        my_data = ss.subset_relation_data(self.data,coll.data)
        return my_data

    def read_topo(self):

        # print "entering  read_topo "

        rel_data = ss.get_topo_rel_data(self.data)
        a_data = rel_data["abuf"]
        if (not a_data): return a_data,a_data
        a_size = len(a_data)
        b_data = rel_data["bbuf"]
        b_size = len(b_data)
        row_set_name = rel_data["sub_set_name"]
        row_name = rel_data["sub_cat_name"]
        col_set_name = rel_data["sup_set_name"]
        col_name = rel_data["sup_cat_name"]

        # Calculate to max number of colums and rows
        ncols = 0
        for value in a_data:
           if (value > ncols): ncols = value
        nrows = b_size/ncols
        # print "nrows",nrows
        # print "ncols",ncols

        # Build an array to store the max number of columns per row 
        columns=[]
        if (a_size == 1):
            for i in range(0,nrows):
                columns.append(ncols)
        else:
            for value in a_data:
                columns.append(value)
           
        # print "columns",columns 
        
        sys.stdout.write("\n%s:%s -> %s:%s\n"%(row_set_name,row_name,col_set_name,col_name))
        sys.stdout.write("%12s"%"")
        for col in range(0,ncols):
             sys.stdout.write("%11s"%col_name)

        j=0
        row = 0
        for ncols in columns:
            print ""
            sys.stdout.write("%s %6d:"%(row_name,(row+1)))
            row=row+1
            for col in range(0,ncols):
                value = b_data[j]
                sys.stdout.write("%10d "%value)
                j=j+1
        print ""

                
        # print "exiting  read_topo "
        return a_data,b_data

class Suite(Set):
    def __init__(self,refdict):
        self.__dict__['_is_suite'] = True;
        Set.__init__(self,refdict)
        self.states = self.fields
        
    def describe(self):
        
        if self.data == {}:
            return
        
        keys = self.__dict__.keys()

        for key in ['name','fields']:

            if key == 'name':
                print "\tname:\t\t",self.data[key]
            elif key == 'fields':
                self.my_states()

            elif key[0] != '_':
                if len(key) > 8:
                    print "\t",key,":\t",self.data[key]
                else:
                    print "\t",key,":\t\t",self.data[key]
                    
    def my_states(self,arg = None):
        if self.fields.data == []:
            return
        
        if isinstance(self.fields, SAFList ):
            if len(self.fields) < 2:
                print "\tstates:\t\t",self.fields[0].name
            else:
                print "\tstates:"
                for i in range(0,len(self.fields)):
                    print "\t\t\t",self.fields[i].name


      
class Subset(Set):

    def __init__(self,refdict):
        Set.__init__(self,refdict)

    def describe(self):
        if self.data == {}:
            return
        
        keys = self.__dict__.keys()


        header = "\n\tSubset:   "  + self.subset_of.name + '_' + self.subset_on.name \
              + ' --> ' + self.name + '_' + self.subset_on.name
        print header

        print "\n\tCategory:\t" + self.subset_on.name + '\n'

        # for key in ['name','type','subset_of','subset_on','collections','subsets','fields']:
        for key in ['name','subsets','fields']:
        # for key in ['name','type','collections','subsets']:
        
            if key == 'collections':
                self.my_collections()
            elif key == 'name':
                print "\tSet:\t\t",self.data[key]
            elif key == 'fields':
                self.my_fields()
            elif key == 'subsets':
                # self.my_subsets()
                if len(self.subsets) >= 1:
                    self.list_names('subsets',self.subsets)
            elif key == 'subset_of':
                if self.subset_of.data != {}:
                    print "\tsubset_of:\t",self.subset_of.name
            elif key == 'subset_on':
                if self.subset_on.data != {}:
                    print "\tsubset_on:\t",self.subset_on.name

            elif key[0] != '_':
                if len(key) > 8:
                    print "\t",key,":\t",self.data[key]
                else:
                    print "\t",key,":\t\t",self.data[key]
                    
    def my_fields(self,arg = None):
        if self.fields.data == []:
            return
        
        if isinstance(self.fields,SAFList):
            
            num = 0
            lookup = []
            for i in range(0,len(self.fields)):
                if self.fields[i].coeff_assoc == self.subset_on.name:
                    lookup.append(i)
                    num += 1
    
            if num > 0 and num < 2:
                print "\tfields:\t\t",self.fields[lookup[0]].name
            elif num > 0:
                print "\tfields:"
                for i in range(0,num):
                    print "\t\t\t",self.fields[lookup[i]].name

    
 
class SAFView(Tkinter.Frame): 
    """ Create a widget display """
    def __init__(self, master=None):
        global _safview_mode
        global _safview_write
        _safview_mode = 1
        _safview_write = self

        win = self
        Tkinter.Frame.__init__(self,master)
        self.createMenu()
        # Tkinter.Label(win, text="Hello Python World").pack(side="top")
        # Tkinter.Button(win, text="Close", command=self.close).pack(side="right")
        # self.createListBox()

        self.createFrames()
        self.createTree()
        # self.createWidgets()
        self.createFieldTextFrame()

        win.pack(fill=Tkinter.BOTH, expand=Tkinter.YES)
        

    def createFrames(self):
        # top frame
        win = self
        self.top_frame = Tkinter.Frame(win)
        self.top_frame.pack(side=Tkinter.TOP, fill=Tkinter.BOTH, expand=Tkinter.YES,)
 
        # bot frame
        self.bot_frame = Tkinter.Frame(win, borderwidth=5, relief=Tkinter.RIDGE, height=50, background="white")
        self.bot_frame.pack(side=Tkinter.TOP, fill=Tkinter.BOTH, expand=Tkinter.YES,)

        # left frame (Tree Frame)
        self.tree_frame = Tkinter.Frame(self.top_frame,
                                      borderwidth=5, relief=Tkinter.RIDGE, height=500, width=50, background="red")
        self.tree_frame.pack(side=Tkinter.LEFT, fill=Tkinter.BOTH, expand=Tkinter.YES)
        # self.tree_frame_left = Tkinter.Frame(self.tree_frame, height=500, width=50)
        # self.tree_frame_left.pack(side=Tkinter.LEFT, fill=Tkinter.BOTH, expand=Tkinter.YES)

        # right frame
        self.Field_frame = Tkinter.Frame(self.top_frame,
                                        borderwidth=5, relief=Tkinter.RIDGE, width=300, background="lightblue")
        self.Field_frame.pack(side=Tkinter.RIGHT, fill=Tkinter.BOTH, expand=Tkinter.YES,)



    def createWidgets(self):
        win = self
        self.QUIT = Tkinter.Button(win)
        self.QUIT["text"] = "QUIT"
        self.QUIT["fg"]   = "red"
        self.QUIT["command"] =  self.close
        
        self.QUIT.pack({"side": "left"})

        self.hi_there = Tkinter.Button(win)
        self.hi_there["text"] = "Hello",
        self.hi_there["command"] = self.say_hi

        self.hi_there.pack({"side": "left"})

    def createMenu(self):
        # create a menu
        mb = Tkinter.Menu(self)
        self.master.config(menu=mb)

        filemenu = Tkinter.Menu(mb)
        mb.add_cascade(label="File", menu=filemenu)
        # filemenu.add_command(label="New", command=self.callback)
        # filemenu.add_command(label="Open...", command=self.callback)
        # filemenu.add_separator()
        # filemenu.add_command(label="Foreground", command=self.quit)
        filemenu.add_command(label="Exit", command=self.close)
        # filemenu.add_command(label="Exit", command=self.exitcallback)

        # helpmenu = Tkinter.Menu(mb)
        # mb.add_cascade(label="Objects", menu=helpmenu)
        # helpmenu.add_command(label="topsets", command=self.topsets)
        # helpmenu.add_command(label="allsets", command=self.allsets)

        # helpmenu = Tkinter.Menu(mb)
        # mb.add_cascade(label="Help", menu=helpmenu)
        # helpmenu.add_command(label="About...", command=self.callback)

    def createListBox(self):
        global _db
        global _topsets
        # self.data = _topsets.__dict__.keys()
        #self.data = {"123": 1,"asdfasdfasdf":23,"adsfasdfsdf":567}.keys()
        # self.data = _topsets.__dict__.keys()
        self.data = _db.keys()
        frame = self
        scrollbar = Tkinter.Scrollbar(frame) #, orient="vertical")
        self.listbox = Tkinter.Listbox(frame, yscrollcommand=scrollbar.set,selectmode="single")
        listbox=self.listbox
        scrollbar.config(command=listbox.yview) 

        self.mylist=[]
        for item in self.data:
            listbox.insert("end",item)
            self.mylist.append(item)  

        listbox.bind("<Double-Button-1>", self.ListBoxSelection) # map event 
        scrollbar.pack(side="right", fill="y")
        listbox.pack(side="left", fill="both", expand=1)

    def ttypager(self,buf):
        textw = self.field_text_area
        textw.insert(Tkinter.END,buf)
        textw.yview(Tkinter.END)  # move the scroll bar

    def createFieldTextFrame(self):
        global topsets
        command = "topsets"
        try:
          text =  eval(command)
        except:
          text = "command '" + command + "' failed"
        frame = self.Field_frame
        textw = Tkinter.Text(frame)
        self.field_text_area = textw
        # textw.pack(side="left",expand=1,fill="both")
        textw.grid(row=0, column=0, sticky='nsew')
                                                                                                                   
                                                                                                                   
    # make expandable
        frame.grid_rowconfigure(0, weight=1)
        frame.grid_columnconfigure(0, weight=1)


        sb=Tkinter.Scrollbar(frame)
        # sb.pack(side="left",expand=1,fill="y")
        sb.grid(row=0, column=1, sticky='ns')
        textw.configure(yscrollcommand=sb.set)
        sb.configure(command=textw.yview)

        self.ttypager(text)


    def exitcallback(self):
        sys.exit(0)

    def ListBoxSelection(self,event_instance):
        items = self.listbox.curselection()
        print "items selected",items
        try:
             items = map(int, items)
        except ValueError: pass
        print "items ",items
        try:
           i = items[0]
           print "items selected",self.mylist[i]
        except: pass

    def single_click_file(self,node):
        path=apply(os.path.join, node.full_id())
        path = path.replace("/",".")
        nicepath = path.replace("'","")
        nicepath = nicepath.replace("]","")
        nicepath = nicepath.replace("__dict__[","")
        try:
            myobject= eval(path)
            if (path.find("allsets")!=0): # Using Allsets to gather topology info
                print nicepath,":\n",myobject
    
            if isinstance(myobject,Field):
                if (myobject.data_size > 0):
                      data = myobject.read()
                      if (myobject.num_comps > 0):
                            sys.stdout.write("[")
                            for i in range(0,len(myobject.components)):
                                 sys.stdout.write("%s,"%myobject.components[i].name)
                            sys.stdout.write("]:\n")
                      else:
                         sys.stdout.write("%s:\n"%myobject.name)
                      print data
            if isinstance(myobject,Collection):
                      data = myobject.read()
                      print data

            # Using Allsets to gather topology info
            if (isinstance(myobject,Set) and (path.find("allsets.")>-1)):
                      # print path.find("allsets.")
                      # print myobject.data
                      # print myobject.read()
                      myobject.read_topo()
        except: 
                pass

        try:
            sys.stdout._flush()
        except:
            pass


    def getfiles(self,node):
        global _db
        global _topsets
        global _stdout
        global topsets
        global suites
        path=apply(os.path.join, node.full_id())
        # _stdout.write( 'in getfiles path: %s\n'%path)
        # _stdout.write( 'in getfiles node.full_id: %s\n'%node.full_id()[0])
        if (path == ""):
            folder=1
            if (topsets): self.t.add_node(name="topsets", id="topsets", flag=folder)
            if (suites):  self.t.add_node(name="suites", id="suites", flag=folder)
            if (allsets):  self.t.add_node(name="topology", id="allsets", flag=folder)
            return 0
         
        
        # print "path:",path
        path = path.replace("/",".")
        topdict= eval(path)
        # print path,":",topdict
        files=topdict.__dict__.keys() 
        # files.sort()
        folder=1
        if isinstance(topdict,SAFObjectDict):
            found_l = 0
            if (path.find("allsets") == 0):  # using allsets to gather topology info.
                return 0 
            if (topdict.has_key("subsets")):
                if (topdict.subsets and (path.find("collections") == -1)):
                    self.t.add_node(name="subsets", id="subsets", flag=folder)
                    found_l = 1
            if (topdict.has_key("collections")):
                if (topdict.collections):
                    self.t.add_node(name="collections", id="collections", flag=folder)
                    found_l = 1
            if (topdict.has_key("fields")):
                if (topdict.fields):
                   self.t.add_node(name="fields", id="fields", flag=folder)
                   found_l = 1

            if (found_l): #  There are redundant entries for collections, subsets and fields
                return 0
               
        for filename in files:
            name=filename
            known_dict=1
            childobj=topdict.__dict__[filename]
            desc = ""
            if isinstance(childobj,Collection):
               desc = " (Collection)" + "(" + str(childobj.count) + ")"
            elif isinstance(childobj,State):  # must be before Field block
               desc = " (State)"
            elif isinstance(childobj,Field):
               desc = " (Field)" + "(" + str(childobj.data_size) + ")"
            elif isinstance(childobj,State):
               desc = " (State)"
            elif isinstance(childobj,Suite):
               desc = " (Suite)"
            elif isinstance(childobj,Subset):
               desc = " (Subset)"
            elif isinstance(childobj,Set):
                if (path.find("allsets") == 0):  # using allsets to gather topology info.
                    rel_obj = ss.is_there_topo_rel_data(childobj.data)
                    if (not rel_obj):
                        known_dict=0
                else:
                    desc = " (Set)"
            elif isinstance(childobj,SAFdbs):
               desc = " (SAFdbs)"
            elif isinstance(childobj,DataBase):
               desc = " (Database)"
            elif isinstance(childobj,SAFObjectDict):
               desc = " (SAFObjectDict)"
            else:
               known_dict=0
           
            if (known_dict):
                folder = 1
                if isinstance(childobj,Collection):
                    folder = 0
                if isinstance(childobj,Field):
                    if (childobj.num_comps <= 0):
                        folder = 0
                self.t.add_node(name=name + desc, id="__dict__['"+filename+"']", flag=folder)


    def createTree(self):
        root=self.tree_frame
# create the control
        self.t=Tree.Tree(master=root,
                    root_id="",
                    root_label=os.sep,
                    get_contents_callback=self.getfiles,
                    single_click_callback=self.single_click_file,
                    width=300, height=400) # drop_callback=self.getfiles,

        self.t.grid(row=0, column=0, sticky='nsew')
                                                                                                                                 
    # make expandable
        root.grid_rowconfigure(0, weight=1)
        root.grid_columnconfigure(0, weight=1)
                                                                                                                                 
    # add scrollbars
        sb=Tkinter.Scrollbar(root)
        sb.grid(row=0, column=1, sticky='ns')
        self.t.configure(yscrollcommand=sb.set)
        sb.configure(command=self.t.yview)
                                                                                                                                 
        sb=Tkinter.Scrollbar(root, orient=Tkinter.HORIZONTAL)
        sb.grid(row=1, column=0, sticky='ew')
        self.t.configure(xscrollcommand=sb.set)
        sb.configure(command=self.t.xview)
                                                                                                                                 
    # must get focus so keys work for demo
        self.t.focus_set()
    # expand out the root
        self.t.root.expand()



    def topsets(self):
        global _topsets
        print "topsets"
        print _topsets

    def allsets(self):
        print "allsets"
        global _allsets
        print _allsets

    def say_hi(self):
        print "Hello again!!!"

    def close(self):
         sys.exit(0)


class SAFcmd(cmd.Cmd):
    """ Build a command line interpreter """
    symbols = globals()
    # Find all of the callable objects and loop for 
    # their doc strings
#    for n in symbols.keys():
#        c = symbols[n]
#       if (callable(c)):
#           if c.__doc__:
#                 exec """
#def help%s(self):print %s.__doc__
#""" % (n,n)
 
    def __init__(self):
        cmd.Cmd.__init__(self)
        self.prompt_orig = 'safsh'
        self.prompt = "safsh> "
        
       
    def onecmd(self, line):
        # eai  global _pagerout
        # eai sys.stdout = _pagerout
        
        line = line.strip()
        if not line:
            return self.emptyline()
        elif line[0] == '?':
            line = 'help ' + line[1:]
        elif line[0] == '!':
            if hasattr(self, 'do_shell'):
                line = 'shell ' + line[1:]
            else:
                return self.default(line)
        self.lastcmd = line
        i, n = 0, len(line)
        # while i < n and line[i] in self.identchars: i = i+1
        while i < n and line[i] not in ' ': i = i+1
        cmd, arg = line[:i], line[i:].strip()
        if cmd == '':
            return self.default(line)
        else:
            try:
                func = getattr(self, 'do_' + cmd)
            except AttributeError:
                return self.default(line)
            return func(arg)

    def default(self,line):
            """ handle unrecognized commands """
            # try:
            
            # handle lines containing '=' and do the assignment
            global _working_dbs

            if line[0] == '!':
                status = os.system(line[1:])
                return
            
            words = string.split(line,"=",1)
            if len(words) > 1:
                words[0] = words[0].strip()
                rargs = string.split(words[1])
                try:
                    saf_command = getattr(safcmd,'do_' + rargs[0])
                except AttributeError:
                    pass
                else:
                    tmp = saf_command(string.join(rargs[1:]))
                    
                    if isinstance(tmp,DataBase):
                        _working_dbs[tmp.data['name']] = words[0]
                        self.prompt = self.prompt_orig + '_' + words[0] + '> '
                        sys.ps1.data = sys.ps1.prompt_orig + '_' + words[0] + '# '
                        tmp._set_globals()
                        
                    stmnt = 'global ' + words[0] + ' ; ' + words[0] + ' = tmp'
                    exec(stmnt)
                    return
                
                stmnt = line
                try:
                    lval = globals()[words[0]]
                except KeyError:
                    try:
                        junk = eval(words[0])

                    except KeyError:
                        stmnt = line
                    except AttributeError:
                        stmnt = line
                    except IndexError:
                        stmnt = line
                    except NameError:
                        # stmnt = "global " + words[0] + " ; " + words[0] + "=" + words[1]
                        stmnt = words[0] + '=' + words[1]
                # else:
                    # stmnt = 'global ' + words[0] + ' ; ' + stmnt

                try:
                    stmnt = "global " + words[0] + ' ; ' + stmnt
                    exec(stmnt)
                except StandardError:
                    stmnt = string.split(stmnt,';',1)[1]
                    print "Could not execute command:",stmnt
                else:
                    return

            # not an assignment, eval the line
            words = string.split(line," ",1)

            # be nice and do the print command if requested
            if words[0] == "print":
                # self.default(words[1])
                self.onecmd(words[1])
                return

            try:
                the_var = eval(line)
            except KeyError, e:
                print "Attribute error:",e
                return
            except IndexError, e:
                print "Index error:",e
                return
            except StandardError:
                # the eval failed, pass to the OS below
                pass
            else:
                # the eval above worked, so do it
                if type(the_var) != type(None):
                    if isinstance(the_var,SAFObjectDict) or isinstance(the_var,SAFList):
                        the_var.describe()
                    else:
                        print the_var
                return 

            # eval on 'line' failed, pass line to the os for a possible OS command
            if line != 'topsets' and line != 'allsets':
                try:
                    status = os.system(line)
                except OSError:
                    print "could not execute: " + line

##            if status != 0:
##                print "OS command return non-zero status: " + line

        # except StandardError:
            # print "could not evaluate statement"
            # return

    def preloop(self):
        # eai global _stdout
        # eai sys.stdout = _stdout
        pass
        
    def precmd(self, line):
        return line


    def postcmd(self,stop,line):
        global _stdout
        global _expert_mode
        try:
            sys.stdout._flush()
        except:
            pass
        # eai sys.stdout = _stdout
        if isinstance(stop,SAFList) or isinstance(stop,SAFObjectDict):
            return 0
        if stop == "expert":  # triggered by do_expert 
            _expert_mode=1
            return 1  # returning 1 here will exit cmdloop
        return 0


    def cccdo_subset(self,line,set=None,coll=None):  
		# don't know what the intention was; looks like set.colletion.name
        if isinstance(line,type("")):
            if line == "":
                return
        if isinstance(line,Set):
            parent = line
            name = set
            collection = coll
        else:
            args = string.split(line)

            parent = eval(args[0])
            name = args[1]
            try:
                collection = args[2]
            except IndexError:
                collection = parent.collections[0].name
        
        return_set = None
        final_set = None
        for i in range(0,len(parent.subsets)):
            if parent.subsets[i].name == name:
                return_set = parent.subsets[i]
                if return_set.subset_on.name == collection:
                    final_set = return_set
    
        if isinstance(final_set,Set):
            return_set = final_set

        if isinstance(line,type("")):
            print return_set
            return
        
        return return_set
                
    def emptyline(self):
        return

    def help_allsets(self):
        string="""
        allsets
            List of sets created with the 'open' command.  The list
            can be traversed using python syntax
            Example:
              allsets[0]
              allsets.CELL_1 
        """
        print string
        return

    def help_topsets(self):
        string ="""
        topsets
          List of topsets created with the 'open' command. The list can be traversed using python syntax 
          Examples:
              topsets[0] 
              topsets.TOP_CELL 
        """
        print string
        return

    def help_suites(self):
        string ="""
        suites
           List of suites created with the 'open' command. The list can be traversed using python syntax 
           Examples:
              suites[0] 
              suites.OTHER_SUITE 
        """
        print string
        return


    def help_help(self):
        print "A single help will due"
        return

    def do_os(self,line):
	"""
	os <command>
		Allow the Operating System to process <command>
	"""
        if line != "":
            os.system(line)
        else:
            return

    def do_expert(self,line):
	"""
	expert
		Enable expert Safsh mode. Allows user to enter a python interactive session. To return
		to Safsh mode the cmdmode() command must be entered.
	"""
        print ""
        print "enabling expert python mode, 'cmdmode()' will return to command mode"
        print ""
        return "expert"   # In cmdloop: onecmd sets stop=expert; then calls postcmd with stop arg; then exits cmdloop

   
    def do_pwd(self,line):
	"""
	pwd
		Print the name of the current working directory.
		Use the 'cd' command to change the current working directory.
	"""
        print _working_dir

    def do_ls(self,line):
	"""
	ls 
		perform a directory listing of the current working directory
	"""
        # os.system("ls " + line + " " + _working_dir)
        os.system("ls " + line )

    def do_cd(self,line=""):
	"""
	cd <dirname>
		Change working directory
	"""
        if line == "":
            dir = os.environ['HOME']
            os.chdir(dir)
            pfile = os.popen('pwd')
            newdir = pfile.read()[:-1]
            pfile.close() ; del pfile
            os.environ['PWD'] = newdir
            globals()['_working_dir'] = newdir

        else:
            try:
                os.chdir(line)
            except OSError:
                print "could not cd to " + line
            else:
                pfile = os.popen('pwd')
                newdir = pfile.read()[:-1]
                pfile.close() ; del pfile
                os.environ['PWD'] = newdir
                globals()['_working_dir'] = newdir
                

		
    def do_open(self,line):
	"""
	open <SafDatabaseFileName>
		open the given SAF Database
	"""
        if line == "":
            line = "test_saf.saf"

        # global _db1
        global _num_open_dbs
        global _db
        global _alldbs
        global _pagerout
        global mystdout
        global _stdout
        global _debug
        global _orig_list_db
        global _prim_saf_file_name


        _pagerout = _stdout
        
        _orig_list_db = ss._saf_open(_working_dir,line)
        if (_debug): traverse(_orig_list_db,0)

        if _orig_list_db == None:
            print "\n Open failed, file name: ",line
            return


        _num_open_dbs += 1
        dbstring = "db%d" % _num_open_dbs
        
        _db = DataBase(_orig_list_db)
        globals()[dbstring] = _db
        globals()[dbstring]._set_globals()

        if _alldbs == None:
            _alldbs = []
            _alldbs = CollectionList(_alldbs)

        _alldbs.append(globals()[dbstring])
        
        print "\nSAF database " + _db['name'] + " opened:\n"
        _prim_saf_file_name = line

        _db.SierraStats()
        
        _pagerout = mystdout
        
        return _db

    def do_setdb(self,line):
	"""
	setdb <databasename>
		Swith to another saf file 
		Example:
			dbs   # show a listing all working databases
			setdb db1
			setdb db2
	"""		
        try:
           tmpdb = eval(line)
           tmpdb._set_globals()
   
           print "set current db to " + line
        except:
           print "  Please use correct syntax: "
           print "       setdb db1"
           print "       setdb db2"

    def do_browse(self,line):
        """
        browse  <SafDatabaseFileName>
                Gives the user the option to view a database using a GUI interface (point & click).
                The database name can be passed although it is not necessary if the database has 
                already been opened with the 'open' command.
                Example:
                        browse
        """

        global _pagerout
        global _stdout
        global _safview_mode
        global _prim_saf_file_name

        if (line): db = self.do_open(line)
        try:
            garb = _db._topsets # is database open
        except:
            print " The database may not be opened correctly. Please check file name or command syntax."
            print "    Syntax:  "
            print "        browse file  "
            return 0
        sys.stdout = _pagerout  # redirect stdout to _pagerout
        view=SAFView()
        view.master.title("SAF View: %s"%_prim_saf_file_name)
        # view.master.maxsize(1000, 400)
        try:
           view.mainloop()
        except:
           pass
        sys.exit(0)  # when we exit the SAFVIew class make sure we exit the application
        sys.stdout = _stdout
        _safview_mode = 0

    def do_dbs(self,line=""):
	"""
	Display working databases
	"""
        print _working_dbs

    def do_ntopsets(self,line):
	"""
	ntopsets
		Show all topsets
	"""
        try:
            print _db._topsets
        except:
            print None
            
    def do_nallsets(self,line):
	"""
	nallsets
		Show all sets
	"""
        try:
            print _allsets
        except:
            print None
           
    def do_sil(self,line=""):
        """
	sil [database_handle]
		prints the Subset Inclusion Lattice Tree (sil) for the
		database represented by the given database handle

		If no argument is given, sil will default to the most
		recently opened database
		
		examples:
			sil
			sil db1
        """
        global _db
        if line == "":
            try:
                # dbhandle = globals()['db1']
                dbhandle = _db
            except KeyError:
                print "no open database"
                return
            else:
                self.ss_recurse(dbhandle)
                return
        try:
            self.ss_recurse(eval(line))
        except StandardError:
            print "Could not evaluate:",line
    def do_describe(self,line):
	"""
	describe object
		List the SAF attributes for the given object
	"""
        self.default(line)
        
    def do_set(self,line):
        """
        Sets a variable to a value
			set <variable> <value>
				Where value can be any python object
        """
        words = string.split(line," ",1)
        stmt = words[0] + " = " + words[1]
        return self.default(stmt)
            
    def ss_recurse(self,db):

        if isinstance(db,DataBase):
            the_db = db.data
        else:
            the_db = _db


        try:
            the_type = the_db['type']
        except NameError:
            print "invalid type, could not determine object type:" ; print db
            return
        except TypeError:
            the_type = type(the_db)
            print "invalid type:",the_type
            print "argument needs to be a SAF database object"
            return
        else:
            if the_type != 'SAF_Db':
                the_name = the_db['name']
                print "invalid type:",the_name,"is type:",the_type
                print "argument needs to be type SAF_db"

            else:
                ss.sil(the_db)
                
    def do_quit(self, line):
        """ 
		Exits the program
		 """
	print "\n"
        sys.exit()

    def do_EOF(self, line):
        "Ctrl-D exits program"
        self.do_quit(line)

    def do_exit(self, line):
        "Exits program"
        self.do_quit(line)


def saf_open(filename, dbname=""):
    try:
        if dbs:
            num_open_dbs = len(dbs) + 1
        else:
            num_open_dbs = 1
    except:
        num_open_dbs = 1
        
    if dbname == "":
        dbname = "db" + str(num_open_dbs)
        local_dbname = 1
    else:
        local_dbname = 0

    the_command = dbname + " = " + "open " + filename
        
    safcmd.default(the_command)
    
    the_db = globals()[dbname]
        
    if the_db:
        if local_dbname == 1:
            the_db._set_globals()

        dbname = the_db
        return dbname
    else:
        print "could not open " + filename
        return None

def subset_list(my_set,level=0):

    if level == 0:

        print "Set" + 20*" " + "Subset" + 15*" " + "Subset_On" + 7*" " + "Subset_Relation_Size"
        print "-------------------------------------------------------------------------"

    for colls in my_set.collections:
        if colls.subsets:
            for subs in colls.subsets:
                print my_set.name,(20-len(my_set.name))*" ", subs.name,\
                      (20-len(subs.name))*" ",colls.name,(15-len(colls.name))*" ", subs.subset_on.count
                subset_list(subs,level+1)

def field_list(the_set):
    if the_set.fields:
        print "Fields on Set: ", the_set.name
        print "   Field_name        Field Data Size"
        print "   ---------------------------------"
        for the_field in the_set.fields:
            print "   ",the_field.name,(20-len(the_field.name))*" ",the_field.data_size
        print "\n"

def traverse(item,n):
        if (type(item) is types.ListType):
            print "list["
            for value in item:
                 if (type(value) is types.DictType) or (type(value) is types.ListType):
                     traverse(value,n+1)
                 else:
                     for i in range(0,n):
                          _stdout.write("    ")
                     print value
            for i in range(0,n):
                  _stdout.write("    ")
            print "]"
        elif (type(item) is types.DictType):
            for i in range(0,n):
                _stdout.write("    ")
            _stdout.write("Dict{")
            for key in item.keys():
               print""
               for i in range(0,n):
                   _stdout.write("    ")
               _stdout.write(str(key))
               _stdout.write(": ")
               traverse(item[key],n+1)
            print "}"
        else:
            _stdout.write(str(item))

    
    
#########################################################
def main():
    global _stdout
    global _db
    global _top
    global _tops
    global _num_top_sets
    global _topsets
    global _allsets
    global _working_dbs
    global dbs
    global _alldbs
    global _pagerout
    global mystdout
    global safcmd
    global cmdmode
    global _debug
    global _expert_mode
    global _stdin
    global _stdin_fileno
    global _stderr
    global _num_open_dbs
    global _orig_list_db
    global _safview_mode
    global _safview_write

    print "starting safsh..."

    _debug        = 0
    _expert_mode  = 0
    _safview_mode = 0
    _safview_write= None
    _stdin        = sys.stdin
    _stdin_fileno = sys.stdin.fileno()
    _stdout       = sys.stdout
    _stderr       = sys.stderr
    _num_open_dbs = 0
    _orig_list_db = {}
    _db           = None
    _alldbs       = None

    _working_dir  = os.environ.get("PWD",os.environ.get("HOME"))
    mystdout      = Mystdout()
    _pagerout     = _stdout


    sys.ps1       = ExpertPrompt("safsh")

    _working_dbs  = SAFdbs({})
    dbs           = _working_dbs

    safcmd        = SAFcmd()
    cmdmode       = safcmd.cmdloop
    safcmd.do_cd(_working_dir)

    # if len(sys.argv) > 1:
    #   _saf_file = sys.argv[1]
    #   db1 = safcmd.do_open(_saf_file)
    #   if db1:
    #       db1._set_globals()
    # else:
    _topsets      = None
    _tops         = None
    _top          = None
    _num_top_sets = 0
    _db           = None
    _allsets      = None
    _alldbs       = None

    if __name__ == '__main__':
        # Execute cmdloop forever until expert mode is entered, then the user is on his own.
        while (not _expert_mode):
            try:
                cmdmode()
            except KeyboardInterrupt,e:
                print "\n Ignoring KeyboardInterrupt Exception \n"

import MA
from MA import *
if (0):  # set to 1 to enable debugger
   debub= pdb.Pdb()
   debub.runcall(main)
else:
   main()
