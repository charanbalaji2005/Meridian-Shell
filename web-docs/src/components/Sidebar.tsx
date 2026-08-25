import React, { useState } from 'react';
import { NAV_STRUCTURE } from '../data/docsNav';
import { ChevronDown, ChevronRight, Compass, Terminal, Image, Layout, Layers, Settings, Package, Code, BookOpen, Info } from 'lucide-react';

interface SidebarProps {
  activeId: string;
  onSelect: (id: string) => void;
  isOpen: boolean;
}

const iconMap: Record<string, React.ReactNode> = {
  Compass: <Compass size={15} />,
  Terminal: <Terminal size={15} />,
  Image: <Image size={15} />,
  Layout: <Layout size={15} />,
  Layers: <Layers size={15} />,
  Settings: <Settings size={15} />,
  Package: <Package size={15} />,
  Code: <Code size={15} />,
  BookOpen: <BookOpen size={15} />,
  Info: <Info size={15} />,
};

export const Sidebar: React.FC<SidebarProps> = ({ activeId, onSelect, isOpen }) => {
  // Open all categories by default
  const [openCategories, setOpenCategories] = useState<Record<string, boolean>>({
    'GETTING STARTED': true,
    'TERMINAL': true,
    'GRAPHICS': true,
    'INTERFACE': true,
    'DEVELOPER': true,
    'CONFIGURATION': true,
    'PACKAGING': true,
    'DEVELOPMENT': true,
    'REFERENCE': true,
    'PROJECT': true,
  });

  const toggleCategory = (title: string) => {
    setOpenCategories((prev) => ({
      ...prev,
      [title]: !prev[title],
    }));
  };

  return (
    <aside className={`meridian-sidebar ${isOpen ? 'open' : ''}`}>
      <div className="sidebar-brand-header">
        <span className="sidebar-doc-label">Meridian Documentation</span>
      </div>

      <nav className="sidebar-scroll-nav">
        {NAV_STRUCTURE.map((category) => {
          const isCategoryOpen = openCategories[category.title] ?? true;
          const hasActiveItem = category.items.some((item) => item.id === activeId);

          return (
            <div key={category.title} className="sidebar-group">
              <button
                className={`sidebar-group-header ${hasActiveItem ? 'group-active' : ''}`}
                onClick={() => toggleCategory(category.title)}
              >
                <span className="group-icon">
                  {iconMap[category.icon] || <Compass size={15} />}
                </span>
                <span className="group-title">{category.title}</span>
                <span className="group-chevron">
                  {isCategoryOpen ? <ChevronDown size={13} /> : <ChevronRight size={13} />}
                </span>
              </button>

              {isCategoryOpen && (
                <ul className="sidebar-items-list">
                  {category.items.map((item) => {
                    const isActive = item.id === activeId;
                    return (
                      <li key={item.id}>
                        <button
                          className={`sidebar-link ${isActive ? 'active' : ''}`}
                          onClick={() => onSelect(item.id)}
                        >
                          <span className="link-bullet"></span>
                          <span className="link-text">{item.title}</span>
                          {item.status === 'development' && (
                            <span className="badge-tag badge-dev">DEV</span>
                          )}
                          {item.status === 'experimental' && (
                            <span className="badge-tag badge-exp">EXP</span>
                          )}
                          {item.status === 'planned' && (
                            <span className="badge-tag badge-plan">PLAN</span>
                          )}
                        </button>
                      </li>
                    );
                  })}
                </ul>
              )}
            </div>
          );
        })}
      </nav>

      <div className="sidebar-footer-panel">
        <a
          href="https://github.com/charanbalaji2005/Meridian-Shell"
          target="_blank"
          rel="noreferrer"
          className="footer-repo-link"
        >
          <span>charanbalaji2005/Meridian-Shell</span>
        </a>
      </div>
    </aside>
  );
};
