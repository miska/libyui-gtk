/********************************************************************
 *           YaST2-GTK - http://en.opensuse.org/YaST2-GTK           *
 ********************************************************************/

/* A simplification of libzypp's API.

   To get a list of packages, setup a Query object and create a PkgQuery with
   it. Package has a set of manipulation methods, the results of which are
   then reported to PkgList listeners, which you can choose to act on your
   interface, if you want them reflected on the viewer.
   Iterate PkgList using integers (it's actually a vector).

   You must register an object that implements Interface, as some transactions
   are bound by user decisions. Ypp is a singleton; first call to get() will
   initialize it; you should call finish() when you're done to de-allocate
   caches.
*/

#ifndef ZYPP_WRAPPER_H
#define ZYPP_WRAPPER_H

#include <string>
#include <list>

enum MarkupType {
	HTML_MARKUP, GTK_MARKUP, NO_MARKUP
};

struct Ypp
{
	struct Repository;

	// Utilities
	struct Node {
		std::string name, order;
		const char *icon;
		Node *next();
		Node *child();
		void *impl;
	};

	// Entries
	struct Package {
		enum Type {
			PACKAGE_TYPE, PATTERN_TYPE, LANGUAGE_TYPE, PATCH_TYPE, TOTAL_TYPES
		};

		Type type() const;
		const std::string &name() const;
		const std::string &summary();
		Node *category();
		Node *category2();
		bool containsPackage (const Ypp::Package *package) const;
		bool fromCollection (const Ypp::Package *collection) const
		{ return collection->containsPackage (this); }
		void containsStats (int *installed, int *total) const;

		std::string description (MarkupType markup);
		std::string filelist (MarkupType markup);
		std::string changelog();
		std::string authors (MarkupType markup);
		std::string support();
		std::string supportText (MarkupType markup);
		std::string size();
		std::string icon();
		bool isRecommended() const;
		bool isSuggested() const;
		int buildAge() const;  // if < 0 , unsupported or error
		bool isSupported() const;
		int severity() const;
		static std::string severityStr (int id);

		std::string provides (MarkupType markup) const;
		std::string requires (MarkupType markup) const;

		bool getPropertyBool (const std::string &prop);
		int getPropertyInt (const std::string &prop);
		std::string getPropertyStr (const std::string &prop, MarkupType markup = NO_MARKUP);

		struct Version {
			std::string number, arch;
			const Repository *repo;
			int cmp /* relatively to installed -- ignore if not installed */;
			void *impl;
		};
		const Version *getInstalledVersion();
			// available versions order is not specified
			// however the most recent version is ensured to be placed as nb==0
		const Version *getAvailableVersion (int nb);
		  // convenience -- null if not from repo:
		const Version *fromRepository (const Repository *repo);

		bool isInstalled();
		bool hasUpgrade();
		bool isLocked();

		bool toInstall (const Version **version = 0);
		bool toRemove();
		bool toModify();
		bool isAuto(); /* installing/removing cause of dependency */

		void install (const Version *version);  // if installed, will re-install
		                                        // null for most recent version
		void remove();
		void undo();
		void lock (bool lock);

		bool canRemove();
		bool canLock();

		struct Impl;
		Impl *impl;
		Package (Impl *impl);
		~Package();
	};

	// when installing/removing/... a few packages at a time, you should use this pair,
	// so that the problem resolver gets only kicked after they are all queued
	void startTransactions();
	void finishTransactions();

	// Listing
	// this class and all proper subclassed are refcounted
	struct PkgList {  // NOTE: this is actually implemented as a vector
		Package *get (int index) const;
		bool highlight (Ypp::Package *pkg) const;  // applicable to some subclasses only
		int size() const;

		void reserve (int size);
		void append (Package *package);
		void sort (bool (* order) (Package *, Package *) = 0);
		void remove (int index);
		void copy (const PkgList list);  // will only copy entry for which match() == true

		bool contains (const Package *package) const;
		int find (const Package *package) const;  // -1 if not found
		Package *find (const std::string &name) const;

		// NOTE: checks if both lists point to the same memory space, not equal contents
		bool operator == (const PkgList &other) const;

		// common properties
		// NOTE: there is a hit first time one of these methods is used
		bool installed() const;
		bool notInstalled() const;
		bool upgradable() const;
		bool modified() const;
		bool locked() const;
		bool unlocked() const;
		bool canRemove() const;
		bool canLock() const;

		// actions
		// NOTE: can take time (depending on size); show busy cursor
		void install(); // or upgrade
		void remove();
		void lock (bool toLock);
		void undo();

		// a Listener can be plugged for when an entry gets modified
		// -- Inserted and Deleted are only available for particular subclasses
		struct Listener {
			virtual void entryChanged  (const PkgList list, int index, Package *package) = 0;
			virtual void entryInserted (const PkgList list, int index, Package *package) = 0;
			virtual void entryDeleted  (const PkgList list, int index, Package *package) = 0;
		};
		void addListener (Listener *listener) const;
		void removeListener (Listener *listener) const;
		// FIXME: listeners only works for getPackages() lists and PkgQuery ones.

		struct Impl;
		Impl *impl;
		PkgList();
		PkgList (Impl *);  // sub-class (to share refcount)
		PkgList (const PkgList &other);
		PkgList & operator = (const PkgList &other);
		virtual ~PkgList();
	};

	// listing of packages as filtered
	struct QueryBase {
		virtual ~QueryBase() {}
		virtual bool match (Package *) = 0;
	};
	struct QueryAnd : public QueryBase {
		QueryAnd();
		void add (QueryBase *query);

		virtual ~QueryAnd();
		virtual bool match (Package *);
		struct Impl;
		Impl *impl;
	};
	struct QueryOr : public QueryBase {
		QueryOr();
		void add (QueryBase *query);

		virtual ~QueryOr();
		virtual bool match (Package *);
		struct Impl;
		Impl *impl;
	};
	struct QueryProperty : public QueryBase {
		QueryProperty (const std::string &property, bool value);
		QueryProperty (const std::string &property, int value);
		QueryProperty (const std::string &property, const std::string &value, bool case_sensitive, bool whole_word);

		virtual ~QueryProperty();
		virtual bool match (Package *);
		struct Impl;
		Impl *impl;
	};
	struct QueryCategory : public QueryBase {
		QueryCategory (const Node *category, bool category2);

		virtual ~QueryCategory();
		virtual bool match (Package *);
		struct Impl;
		Impl *impl;
	};
	struct QueryRepository : public QueryBase {
		QueryRepository (const Repository *repository);

		virtual bool match (Package *);
		const Repository *repo;
	};
	struct QueryCollection : public QueryBase {
		QueryCollection (const Ypp::Package *collection);

		virtual bool match (Package *);
		const Ypp::Package *collection;
	};

	struct PkgQuery : public PkgList {
		PkgQuery (const PkgList list, QueryBase *query);
		PkgQuery (Package::Type type, QueryBase *query);  // shortcut

		struct Impl;
		Impl *impl;
	};

	struct PkgSort : public PkgList {
		PkgSort (PkgList list, const std::string &prop, bool ascending);
	};

	// list primitives
	const PkgList getPackages (Package::Type type);

	// Resolver
	struct Problem {
		std::string description, details;
		struct Solution {
			std::string description, details;
			bool apply;
			void *impl;
		};
		Solution *getSolution (int nb);
		void *impl;
	};

	// Log
	struct Log {
		struct Entry {
			enum Action { INSTALL_ACTION, REMOVE_ACTION };
			Action action;
			std::string date;
			std::string package_str;
			std::string version_str;

			// attemps to match the strings: could be null
			Package *package;
			const Package::Version *version;  // only may be set for INSTALL
		};

		Entry *getEntry (int nb);

		struct Impl;
		Impl *impl;
		Log();
		~Log();
	};
	Log *getLog();

	// Module
	static Ypp *get();
	static void finish();

	struct Interface {
		virtual bool acceptLicense (Package *package, const std::string &license) = 0;
		virtual void notifyMessage (Package *package, const std::string &message) = 0;
		// resolveProblems = false to cancel the action that had that effect
		virtual bool resolveProblems (const std::list <Problem *> &problems) = 0;
		virtual bool allowRestrictedRepo (const PkgList list) = 0;
		virtual void loading (float progress) = 0;
	};
	void setInterface (Interface *interface);

	// Repositories
	struct Repository {
		std::string name, url, alias /*internal use*/;
		bool enabled;
	};
	const Repository *getRepository (int nb);
	void setFavoriteRepository (const Repository *repo);  /* 0 to disable restrictions */
	const Repository *favoriteRepository();

	// Misc
	Node *getFirstCategory (Package::Type type);
	Node *getFirstCategory2 (Package::Type type);

	struct Disk {
		struct Partition {
			std::string path, used_str, delta_str, free_str, total_str;
			long long used, delta, total;
		};
		const Partition *getPartition (int nb);

		struct Listener {
			virtual void updateDisk() = 0;
		};
		void addListener (Listener *listener);

		Disk();
		~Disk();
		struct Impl;
		Impl *impl;
	};
	Disk *getDisk();

	bool isModified();  // any change made?

	bool importList (const char *filename);
	bool exportList (const char *filename);
	bool createSolverTestcase (const char *dirname);

	Ypp();
	~Ypp();
	struct Impl;
	Impl *impl;
};

#endif /*ZYPP_WRAPPER_H*/
